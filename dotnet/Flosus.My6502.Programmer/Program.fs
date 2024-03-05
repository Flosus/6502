namespace Flosus.My6502.Programmer

open System
open System.Collections.Concurrent
open System.IO
open System.IO.Ports
open System.Threading

module EEPROMProgrammer =
    
    let mutable start = DateTime.Now

    type Context =
        { SerialPort: SerialPort
          mutable IsReady: bool
          mutable IsLastBinary: bool
          mutable Binary: byte array
          BinaryPath: string
          Messages: ConcurrentQueue<byte> }

    let writeFile (ctx: Context) =
        let time = DateTime.Now - start
        let seconds = time.TotalSeconds
        let maxValue = 32768
        let binLength = ctx.Binary.Length
        let binDouble = double binLength
        let speed = binDouble/(double seconds)
        let missing = maxValue - binLength
        let expectedTime = (double missing) / speed
        let percent = Math.Round ((double 100) * binDouble / (double maxValue), 3)
        printfn $"Writing bin; sec={Math.Round (seconds, 2)} length={ctx.Binary.Length} speed={Math.Round (speed, 2)}bytes/s"
        printfn $"{binLength}/{maxValue} = {percent}%%; expectedSeconds={Math.Round (expectedTime, 2)}s"
        File.WriteAllBytes (ctx.BinaryPath, ctx.Binary)

    let waitForReady (port: Context) =
        while not port.IsReady do
            Thread.Sleep 1000
    
    let sendAndWait (port: Context) (cmd:string) =
        waitForReady port
        Thread.Sleep 50
        port.SerialPort.Write($"{cmd}\n")
        port.IsReady <- false
        Thread.Sleep 50
        waitForReady port


    let readPage (port: Context) (size: byte) (address: uint16) =
        let send = sendAndWait port
        send "CMD_R"
        send $"CMD_A{address}"
        send $"CMD_S{size}"
        send "CMD_START"

    let readAll (port: Context) =
        let send = sendAndWait port
        send "CMD_RA"

    let writeAll (ctx: Context) =
        waitForReady ctx
        Thread.Sleep 50
        let data = File.ReadAllBytes ctx.BinaryPath
        ctx.SerialPort.Write("CMD_WA\n")
        //ctx.SerialPort.Write (data, 0, data.Length)
        waitForReady ctx

    let rec read (ctx: Context) =
        if not ctx.SerialPort.IsOpen then
            printfn "No more reading"
        else
            try
                let bt = ctx.SerialPort.ReadByte()
                ctx.IsReady <- false

                match bt with
                | b when b >= 0 && b <= int Byte.MaxValue ->
                    ctx.Messages.Enqueue(byte bt)                    
                    if ctx.Messages.Count % 64 = 0 then
                        let array = ctx.Messages.ToArray ()
                        ctx.Messages.Clear ()
                        
                        let charArray =  array |> Array.map (fun b -> char b)
                        let str = String(charArray)
                        let binEndMsg = "BIN_END"
                        ctx.IsLastBinary <- ctx.IsLastBinary && (not (str.StartsWith binEndMsg) || (str.StartsWith "rdy"))
                        if ctx.IsLastBinary then
                            let ctxBinary = ctx.Binary
                            ctx.Binary <- Array.append ctxBinary array
                            writeFile ctx
                        else 
                            let binStartMsg = "BIN_START"
                            ctx.IsLastBinary <- str.StartsWith binStartMsg
                            if not (str.StartsWith "rdy") then
                                printfn $"{str}"
                            else
                                printfn "rdy"
                                ctx.IsReady <- true
                | i -> printfn $"INVALID READ BYTE: {i}"
            with
            | :? TimeoutException ->
                ctx.IsReady <- true
            | e -> printfn $"e:{e.Message}"

            read ctx


    [<EntryPoint>]
    let main args =
        printfn "Starting up"
        let portName = args[0]
        let action = args[1]
        let binaryPath = args[2]
        let port = new SerialPort(portName, 115200)
        port.ReadTimeout <- 5000

        let ctx: Context =
            { SerialPort = port
              IsReady = false
              IsLastBinary = false
              Binary = [||]
              BinaryPath = binaryPath
              Messages = ConcurrentQueue<byte>() }
        start <- DateTime.Now
        let run () = read ctx
        let thread = Thread(run)
        port.Open()
        thread.Start()

        port.NewLine <- "\n'"
        match action with
        | "read" ->
            printfn "reading data"
            readAll ctx
        | "write" ->
            printfn "writing data"
            writeAll ctx
        | _ -> printfn $"Unknown action {action}"


        printfn "Sleeping"
        Thread.Sleep 2000
        printfn "Waiting"
        ctx.IsReady <- false
        waitForReady ctx
        
        printfn "Closing ports"
        port.Close()
        printfn "Bye bye"
        0
