namespace Flosus.My6502.Lib

open System.IO.Ports
open System.Threading

module SerialManagement =

    type Address = uint16
    let MinAddress: Address = 0x0us
    let MaxAddress: Address = 0x7FFEus
    let MinPageSize = 1
    let MaxPageSize = 64
    type Data = byte

    type Command =
        | WriteByte of Address * Data
        | WritePage of Address * byte[]
        | WriteAll of Data[]
        | ReadByte of Address
        | ReadRange of Address * Address
        | ReadAll

    type CommandResult =
        | InvalidCommand of string
        | Error of string
        | Success
        | SuccessData of byte[]


    type SerialContext = SerialPort
    exception NotReadyError of string
    
    
    let toHex (i: byte): string =
        i.ToString("X")
    
    let toFormattedData data =
        let leftPadHex (str:string) =
            match str.Length with
            | 1 -> "0" + str
            | _ -> str
        data
        |> Array.map toHex
        |> Array.map leftPadHex
        |> Array.toSeq
        |> String.concat " "

    let private isNotInRange min max value = value < min || value > max
    let private IsInvalidAddress = isNotInRange MinAddress MaxAddress

    let private IsInvalidPage (page: byte[]) = isNotInRange 0 0x7FFF page.Length

    let private IsInvalidAllSize (data: Data[]) = data.Length <> (int MaxAddress + 1)

    let private InvalidAddressCommand address =
        InvalidCommand($"Invalid address: {address}")

    let private InvalidDataCommand length =
        InvalidCommand($"Invalid data length: {length}")

    let private WaitFor str (ctx:SerialContext) =
        let line = ctx.ReadLine ()
        printfn $"Got line: {line}"
        match line with
        | l when l = str -> ()
        | _ -> raise (NotReadyError(line))
    
    let private ReadData (startAddress: Address) (size: int) (ctx: SerialContext) =
        printfn $">>>Reading @{startAddress} + {size}"
        WaitFor "READY" ctx
        ctx.WriteLine "READ\n"
        WaitFor "READ" ctx
        ctx.WriteLine $"{startAddress}\n"
        WaitFor (string startAddress) ctx
        ctx.WriteLine $"{size}\n"
        WaitFor (string size) ctx
        let buffer: byte array = Array.zeroCreate size
        Thread.Sleep 100
        let r = ctx.Read(buffer, 0, size)
        let bytes = buffer[0 .. r - 1]
        Thread.Sleep 100
        printfn $">>>Finished reading: {bytes.Length}"
        printfn $">>> HEX: \"{toFormattedData bytes}\""
        Thread.Sleep 100
        let str = System.Text.Encoding.ASCII.GetString bytes
        printfn ">>>RAW:v"
        printfn $"\"{str}\""
        printfn ">>>RAW:^"
        SuccessData(bytes)

    let private ReadFromTo (startAddress: Address) (endAddress: Address) (ctx: SerialContext) =
        let readChunk (indexes: int array) =
            let startChunkIdx = startAddress + uint16 indexes[0]
            Thread.Sleep 1000
            let result = ReadData startChunkIdx indexes.Length ctx

            match result with
            | SuccessData(data) -> data
            | _ -> [||]

        let additionalBytes = endAddress - startAddress

        seq { 0..1 .. int additionalBytes }
        |> Seq.toArray
        |> Array.chunkBySize 64
        |> Array.map readChunk
        |> Array.concat
        |> SuccessData

    let private WriteByte (address: Address) (data: byte) (ctx: SerialContext) =
        ctx.WriteLine "WRITE\n"
        ctx.WriteLine $"{address}\n"
        ctx.WriteLine $"{1}\n"
        ctx.Write([| data |], 0, 1)
        Success

    let private WriteByteArray (address: Address) (data: byte[]) (ctx: SerialContext) =
        ctx.WriteLine "WRITE\n"
        ctx.WriteLine $"{address}\n"
        ctx.WriteLine $"{data.Length}\n"
        Thread.Sleep 100
        ctx.Write(data, 0, data.Length)
        Success

    let private WriteBytes (startAddress: Address) (data: byte[]) (ctx: SerialContext) =
        let sendBytes (indexes: int array) =
            let address = startAddress + uint16 indexes[0]
            let slice = data[indexes[0] .. indexes[indexes.Length - 1]]
            Thread.Sleep 100
            WriteByteArray address slice ctx

        seq { 0..1 .. data.Length }
        |> Seq.toArray
        |> Array.splitInto 64
        |> Array.map sendBytes
        |> ignore

        Success

    let Send (cmd: Command) (ctx: SerialContext) : CommandResult =
        match cmd with
        | ReadByte(address) when IsInvalidAddress address -> InvalidAddressCommand address
        | ReadByte(address) -> ReadData address 1 ctx
        | ReadRange(add, _) when IsInvalidAddress add -> InvalidAddressCommand add
        | ReadRange(_, add) when IsInvalidAddress add -> InvalidAddressCommand add
        | ReadRange(addFrom, addTo) -> ReadFromTo addFrom addTo ctx
        | ReadAll -> ReadFromTo MinAddress MaxAddress ctx
        | WriteByte(address, _) when IsInvalidAddress address -> InvalidAddressCommand address
        | WriteByte(address, data) -> WriteByte address data ctx
        | WritePage(address, _) when IsInvalidAddress address -> InvalidAddressCommand address
        | WritePage(_, page) when IsInvalidPage page -> InvalidDataCommand page.Length
        | WritePage(address, data) -> WriteBytes address data ctx
        | WriteAll data when IsInvalidAllSize data -> InvalidDataCommand data.Length
        | WriteAll data -> WriteBytes MinAddress data ctx
