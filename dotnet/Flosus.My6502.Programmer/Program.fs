

namespace Flosus.My6502.Programmer

open System.IO
open System.IO.Ports
open Flosus.My6502.Lib
open Flosus.My6502.Lib.SerialManagement

module EEPROMProgrammer =


    [<EntryPoint>]
    let main args =
        printfn "Starting up"
        let ctx: SerialContext = new SerialPort("COM8", 9600)
        ctx.Open()
        //let command = ReadAll
        //let command = ReadRange (MinAddress, MinAddress + uint16 511)
        let command = ReadRange (MinAddress, MinAddress + uint16 0x3F)
        let bytes = [| 0 .. 1 .. int MaxAddress|]
                    |> Array.map (fun i -> byte (i % 255))
        //let command = WriteAll bytes
        let dataResult = Send command ctx
        match dataResult with
        | SuccessData data -> File.WriteAllBytes ("./eeprom.bin", data)
        | _ -> printfn "oh no"
        printfn "Closing context"
        ctx.Close()
        printfn "Bye bye"
        0
        