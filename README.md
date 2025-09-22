
# Network Memory Accessor for Super Paper Mario
A mod that allows for memory reading and writing over the network.

#### Note : Current supported versions: eu0, us2, jp1

## How To Use

Once this mod is running, your private IP address will be displayed on the title screen. You can use a script such as **./netmemoryaccess_client.py** to send commands with the appropriate syntax. You can find a list of commands in the list below.

## List of commands

### Read
Syntax: `read address n`

Example: `read 0x80000000 6` will return the game ID.

Returns the bytes at the specified address.

### Write
Syntax: `write address base64_encoded_bytearray decoded_data_size`

Example: `write 0x80000000 SGVsbG8gV29ybGQ= 11` will write "Hello World" to 0x80000000.

Returns the number of bytes written.

### Msgbox
Syntax: `msgbox base64_encoded_message decoded_message_size`

Example: `msgbox PHN5c3RlbT48cD5IZWxsbyBXb3JsZDxrPjxvPg== 28` will spawn a system message box saying "Hello World".

Returns the number of bytes in the message.

#### Note : The message argument uses SPM message markdown. The decoded data in the above example says `<system><p>Hello World<k><o>` (which is 28 characters long).

## Credits

* **PistonMiner**, **Zephiles** and **Seeky** for the SPM rel loader.
* **bushing** for network_wii.c
* **The FreeRTOS team** for their work on coreHTTP and coreJSON.
* **Electronic Arts Inc.** for the EA Standard Library (EASTL)

