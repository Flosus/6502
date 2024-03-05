namespace Flosus.My6502.Programmer

open System
open System.Collections.Concurrent
open System.IO
open System.IO.Ports
open System.Threading
open Flosus.My6502.Lib
open Flosus.My6502.Lib.SerialManagement

module EEPROMProgrammer =
    
    let mutable start = DateTime.Now
    

    type Context =
        { SerialPort: SerialPort
          mutable IsReady: bool
          mutable IsLastBinary: bool
          mutable Binary: byte array
          Messages: ConcurrentQueue<byte> }

    let writeFile (ctx: Context) =
        let time = DateTime.Now - start
        let seconds = time.TotalSeconds
        let maxValue = UInt16.MaxValue
        let binLength = ctx.Binary.Length
        let binDouble = double ctx.Binary.Length
        let speed = binDouble/(double seconds)
        let missing = maxValue - (uint16 binLength)
        let expectedTime = (double missing) / speed
        let percent = Math.Round ((double 100) * binDouble / (double maxValue), 3)
        printfn $"Writing bin; sec={seconds} length={ctx.Binary.Length} speed={speed}bytes/s"
        printfn $"{binLength}/{maxValue} = {percent}%%; expectedSeconds={Math.Round (expectedTime, 2)}s"
        File.WriteAllBytes ("G:\\dev\\6502\EEPROM.BIN", ctx.Binary)

    let waitForReady (port: Context) =
        while not port.IsReady do
            ()
    
    let sendAndWait (port: Context) (cmd:string) =
        waitForReady port
        Thread.Sleep 100
        port.SerialPort.Write($"{cmd}\n")
        Thread.Sleep 100
        port.IsReady <- false
        waitForReady port


    let readPage (port: Context) (size: byte) (address: uint16) =
        let send = sendAndWait port
        // printfn "Sending CMD_R"
        send "CMD_R"
        // printfn $"Sending CMD_A{address}"
        send $"CMD_A{address}"
        // printfn $"Sending CMD_S{size}"
        send $"CMD_S{size}"
        // printfn $"Sending CMD_START"
        send "CMD_START"

    let readAll (port: Context) =
        let size: byte = byte 64
        [|0 .. 511|]
        |> Array.iter (fun i ->
                readPage port size ((uint16 i) * (uint16 64))
            )

    let rec read (ctx: Context) =
        if not ctx.SerialPort.IsOpen then
            printfn "No more reading"
        else
            try
                let bt = ctx.SerialPort.ReadByte()

                match bt with
                | b when b > 0 && b <= int Byte.MaxValue ->
                    ctx.Messages.Enqueue(byte bt)                    
                    if ctx.Messages.Count % 64 = 0 then
                        let array = ctx.Messages.ToArray ()
                        ctx.Messages.Clear ()
                        if ctx.IsLastBinary then
                            let ctxBinary = ctx.Binary
                            ctx.Binary <- Array.append ctxBinary array
                            writeFile ctx
                            ctx.IsLastBinary <- false
                        else 
                            let charArray =  array |> Array.map (fun b -> char b)
                            let str = String(charArray)
                            let binStartMsg = "BIN_START0000000000000000000000000000000000000000000000000000000"
                            ctx.IsLastBinary <- str = binStartMsg
                            if not (str.StartsWith "rdy") then
                                printfn $"{str}"
                                
                        ctx.IsReady <- true
                | _ -> printfn "INVALID READ BYTE"
            with
            | :? TimeoutException -> ctx.IsReady <- true
            | e -> printfn $"e:{e.Message}"

            read ctx


    [<EntryPoint>]
    let main args =
        printfn "Starting up"
        let port = new SerialPort("COM5", 9600)
        port.ReadTimeout <- 1000

        let ctx: Context =
            { SerialPort = port
              IsReady = false
              IsLastBinary = false
              Binary = [||] 
              Messages = ConcurrentQueue<byte>() }
        start <- DateTime.Now
        let run () = read ctx
        let thread = Thread(run)
        port.Open()
        thread.Start()

        port.NewLine <- "\n'"
        readAll ctx

        ctx.IsReady <- false
        waitForReady ctx

        Thread.Sleep 2000
        
        printfn "Closing ports"
        port.Close()
        printfn "Bye bye"
        0
