namespace Flosus.My6502.Programmer

open System
open System.Collections.Concurrent
open System.IO
open System.IO.Ports
open System.Threading
open Flosus.My6502.Lib
open Flosus.My6502.Lib.SerialManagement

module EEPROMProgrammer =

    type Context =
        { SerialPort: SerialPort
          mutable IsReady: bool
          Messages: ConcurrentQueue<byte> }


    let waitForReady (port: Context) =
        while not port.IsReady do
            ()
    
    let sendAndWait (port: Context) (cmd:string) =
        waitForReady port
        port.SerialPort.Write($"{cmd}\n")
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
                    printf $"{char bt}"

                    if ctx.Messages.Count % 64 = 0 then
                        printfn ""
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
              Messages = ConcurrentQueue<byte>() }

        let run () = read ctx
        let thread = Thread(run)
        port.Open()
        thread.Start()

        port.NewLine <- "\n'"
        readAll ctx |> ignore

        ctx.IsReady <- false
        waitForReady ctx

        Thread.Sleep 2000
        
        printfn "Closing ports"
        port.Close()
        printfn "Bye bye"
        0
