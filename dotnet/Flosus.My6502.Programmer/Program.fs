namespace Flosus.My6502.Programmer

open System
open System.IO
open System.IO.Ports
open System.Threading
open Flosus.My6502.Lib
open Flosus.My6502.Lib.SerialManagement

module EEPROMProgrammer =


    [<EntryPoint>]
    let main args =
        printfn "Starting up"

        let dumper = DumpMessageDebug "./output.log"

        let ctx: SerialContext =
            CreateNewSerialContext (new SerialPort("COM5", 9600)) PrintMsgDebug

        //let command = ReadAll
        //let command = ReadRange (MinAddress, MinAddress + uint16 511)
        let command = ReadRange(MinAddress, MinAddress + uint16 0x3F)
        let bytes = [| 0..1 .. int MaxAddress |] |> Array.map (fun i -> byte (i % 255))
        //let command = WriteAll bytes
        //let dataResult = Send command ctx

        // match dataResult with
        // | SuccessData data -> File.WriteAllBytes("./eeprom.bin", data)
        // | _ -> printfn "oh no"
        printfn $"EXISTING: {ctx.SerialPort.ReadExisting()}"

        let write cmd =
            let f () =
                ctx.SerialPort.Write $"{cmd}\n"
                ()

            f

        (*runAndWait (write "CMD_A0") ctx
        runAndWait (write "CMD_S64") ctx
        runAndWait (write "CMD_R") ctx
        runAndWait (write "CMD_START") ctx

        runAndWait (write "CMD_R") ctx
        runAndWait (write "CMD_A64") ctx
        runAndWait (write "CMD_S64") ctx
        runAndWait (write "CMD_START") ctx

        runAndWait (write "CMD_R") ctx
        runAndWait (write "CMD_A128") ctx
        runAndWait (write "CMD_S64") ctx
        runAndWait (write "CMD_START") ctx

        runAndWait (write "CMD_R") ctx
        runAndWait (write "CMD_A192") ctx
        runAndWait (write "CMD_S64") ctx
        runAndWait (write "CMD_START") ctx

        runAndWait (write "CMD_R") ctx
        runAndWait (write "CMD_A256") ctx
        runAndWait (write "CMD_S64") ctx
        runAndWait (write "CMD_START") ctx*)
        
        let forEachPage (page: int) =
            let address = page * 64
            printfn $"{page} -> {address}"
            runAndWait (write "CMD_R") ctx
            runAndWait (write $"CMD_A{address}") ctx
            runAndWait (write "CMD_S64") ctx
            runAndWait (write "CMD_START") ctx
            ()
        [0 .. 511]
        |> Seq.toArray
        |> Array.iter forEachPage

        (*Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine
        Console.ReadLine () |> ctx.SerialPort.WriteLine*)

        let rec wait () =
            match ctx.IsExecuting with
            | true -> wait ()
            | false ->
                ctx.IsRunning <- false
                ()

        wait ()
        ctx.SerialPort.Close()
        printfn "Closing context"
        printfn "Bye bye"
        0
