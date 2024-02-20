namespace Flosus.My6502.Lib

open System.Collections.Concurrent
open System.IO
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

    type SerialContext =
        { SerialPort: SerialPort
          MessageBytes: ConcurrentQueue<byte array>
          Messages: ConcurrentQueue<string>
          OnMessageHandler: string -> unit
          mutable IsExecuting: bool
          mutable IsRunning: bool }

    exception NotReadyError of string

    let ToHex (i: byte) : string = i.ToString("X")

    let ToFormattedData data =
        let leftPadHex (str: string) =
            match str.Length with
            | 1 -> "0" + str
            | _ -> str

        data
        |> Array.map ToHex
        |> Array.map leftPadHex
        |> Array.toSeq
        |> String.concat " "

    let runAndWait (exe: unit -> unit) (ctx: SerialContext) =
        ctx.IsExecuting <- true
        exe ()

        let rec wait () =
            match ctx.IsExecuting with
            | true ->
                Thread.Sleep(10)
                wait ()
            | false -> ()

        wait ()
        Thread.Sleep 100

    let write (path: string) (data: byte array) =
        use fs =
            match File.Exists path with
            | false -> File.Create path
            | true -> File.Open(path, FileMode.Append)

        fs.Write(data, 0, data.Length)

    let private TryParseInput (ctx: SerialContext) =
        let allData = ctx.MessageBytes.ToArray() |> Array.concat
        let str = System.Text.Encoding.ASCII.GetString allData
        let endIndex = str.IndexOf "END"

        if str.StartsWith "BIN" && endIndex <> -1 then
            ctx.MessageBytes.Clear()
            let data = allData[3 .. allData.Length - 4]
            write "./EEPROM.BIN" data
            ()
        elif str.StartsWith "RESP" && endIndex <> -1 then
            ctx.MessageBytes.Clear()
            ctx.IsExecuting <- false
            let data = str[4 .. str.Length - 4]
            ctx.OnMessageHandler data
        else
            ()

    let rec ReadPort (ctx: SerialContext) =
        let buffer: byte array = Array.zeroCreate 64
        let readCount = ctx.SerialPort.Read(buffer, 0, 64)

        match readCount with
        | 0 -> ()
        | _ ->
            let bytesRead = buffer[.. readCount - 1]
            ctx.MessageBytes.Enqueue bytesRead
            TryParseInput ctx

        match ctx.IsRunning with
        | false -> ctx.SerialPort.Close()
        | true -> ReadPort ctx

    let PrintMsgDebug (str: string) = printfn $"[Arduino]{str}"

    let SwallowMessage (_: string) = ()

    let DumpMessageDebug (filePath: string) =
        let streamWriter = File.AppendText filePath

        let _DebugDump (str: string) = streamWriter.Write str

        _DebugDump


    let CreateNewSerialContext (port: SerialPort) printMsg : SerialContext =
        let ctx =
            { SerialPort = port
              MessageBytes = ConcurrentQueue<byte array>()
              Messages = ConcurrentQueue<string>()
              OnMessageHandler = printMsg
              IsExecuting = false
              IsRunning = true }

        let run () = ReadPort ctx
        let thread = Thread(run)
        port.Open()
        thread.Start()
        ctx

    let private isNotInRange min max value = value < min || value > max
    let private IsInvalidAddress = isNotInRange MinAddress MaxAddress

    let private IsInvalidPage (page: byte[]) = isNotInRange 0 0x7FFF page.Length

    let private IsInvalidAllSize (data: Data[]) = data.Length <> (int MaxAddress + 1)

    let private InvalidAddressCommand address =
        InvalidCommand($"Invalid address: {address}")

    let private InvalidDataCommand length =
        InvalidCommand($"Invalid data length: {length}")

    let private WaitFor str (ctx: SerialContext) =
        let line = ctx.SerialPort.ReadLine()
        printfn $"Got line: {line}"

        match line with
        | l when l = str -> ()
        | _ -> raise (NotReadyError(line))

    let private ReadData (startAddress: Address) (size: int) (ctx: SerialContext) =
        printfn $">>>Reading @{startAddress} + {size}"
        WaitFor "READY" ctx
        ctx.SerialPort.WriteLine "READ\n"
        WaitFor "READ" ctx
        ctx.SerialPort.WriteLine $"{startAddress}\n"
        WaitFor (string startAddress) ctx
        ctx.SerialPort.WriteLine $"{size}\n"
        WaitFor (string size) ctx
        let buffer: byte array = Array.zeroCreate size
        Thread.Sleep 100
        let r = ctx.SerialPort.Read(buffer, 0, size)
        let bytes = buffer[0 .. r - 1]
        Thread.Sleep 100
        printfn $">>>Finished reading: {bytes.Length}"
        printfn $">>> HEX: \"{ToFormattedData bytes}\""
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
        ctx.SerialPort.WriteLine "WRITE\n"
        ctx.SerialPort.WriteLine $"{address}\n"
        ctx.SerialPort.WriteLine $"{1}\n"
        ctx.SerialPort.Write([| data |], 0, 1)
        Success

    let private WriteByteArray (address: Address) (data: byte[]) (ctx: SerialContext) =
        ctx.SerialPort.WriteLine "WRITE\n"
        ctx.SerialPort.WriteLine $"{address}\n"
        ctx.SerialPort.WriteLine $"{data.Length}\n"
        Thread.Sleep 100
        ctx.SerialPort.Write(data, 0, data.Length)
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
