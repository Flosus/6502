

namespace Flosus.My6502.Programmer

open System.IO
open System.IO.Ports
open Flosus.My6502.Lib
open Flosus.My6502.Lib.SerialManagement

module EEPROMProgrammer =


    [<EntryPoint>]
    let main args =
        printfn "Starting up"
        let ctx: SerialContext = new SerialPort("COM5", 9600)
        ctx.Open()
        //let command = ReadAll
        let command = ReadRange (MinAddress, MinAddress + uint16 512)
        let command = ReadRange (MinAddress, MinAddress + uint16 64)
        let dataResult = Send command ctx
        match dataResult with
        | SuccessData data -> File.WriteAllBytes ("G:\\dev\\6502\\dotnet\\eeprom.bin", data)
        | _ -> printfn "oh no"
        printfn "Closing context"
        ctx.Close()
        printfn "Bye bye"
        0
        