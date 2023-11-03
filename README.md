# Sqlite3 SD_MMC ESP-IDF 5 Example

Uses the SD MMC (4-bit data) port on ESP32 to retrieve from SQLite databases.

For more information, please see README.md at https://github.com/siara-cc/esp32_arduino_sqlite3_lib/

## How to use example

You must copy the files `mdr512.db` and `chinook.db` in the `data` folder of this repository and paste them in the root of the SD Card.

### Hardware Required

You need a ESP32 basic Devboard and a SD Card reader. This was tested in a [ESP32-CAM](https://www.amazon.com/Aideepen-ESP32-CAM-MB-ESP-WROOM-32-Development-Board/dp/B0BKT6TSN7?ref_=ast_sto_dp).

### ESP-IDF Compatibility

For this code to compile with `ESP-IDF v5.X.X`you must:

* In the file `esp/esp-idf/components/esp32-idf-sqlite3/CMakeLists.txt` add `spi_flash` to `REQUIRES`.

For this code to compile with `ESP-IDF v4.X.X`you must copy [esp32-idf-sqlite3 repository](https://github.com/siara-cc/esp32-idf-sqlite3) to your components folder. In this repository are more step by step instructions.

## Example Output
Note that the output, in particular the order of the output, may vary depending on the environment.

    I (463) main_task: Calling app_main()
    I (463) sqlite3_sdmmc: Initializing SD card
    I (463) sqlite3_sdmmc: Using SDMMC peripheral
    I (473) gpio: GPIO[13]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
    Name: SA04G
    Type: SDHC/SDXC
    Speed: 20.00 MHz (limit: 20.00 MHz)
    Size: 3724MB
    CSD: ver=2, sector_size=512, capacity=7626752 read_bl_len=9
    SSR: bus_width=4
    I (523) sqlite3_sdmmc: Opening db chinook
    Opened database successfully
    I (533) sqlite3_sdmmc: Opening db mdr512
    Opened database successfully
    Select * from albums where AlbumId < '10'
    Callback function called: 
    AlbumId = 1
    Title = For Those About To Rock We Salute You
    ArtistId = 1

    Callback function called: 
    AlbumId = 2
    Title = Balls to the Wall
    ArtistId = 2

    Callback function called: 
    AlbumId = 3
    Title = Restless and Wild
    ArtistId = 2

    Callback function called: 
    AlbumId = 4
    Title = Let There Be Rock
    ArtistId = 1

    Callback function called: 
    AlbumId = 5
    Title = Big Ones
    ArtistId = 3

    Callback function called: 
    AlbumId = 6
    Title = Jagged Little Pill
    ArtistId = 4

    Callback function called: 
    AlbumId = 7
    Title = Facelift
    ArtistId = 5

    Callback function called: 
    AlbumId = 8
    Title = Warner 25 Anos
    ArtistId = 6

    Callback function called: 
    AlbumId = 9
    Title = Plays Metallica By Four Cellos
    ArtistId = 7

    Operation done successfully
    Time taken: 177823
    Select * from domain_rank where domain = 'zoho.com'
    Callback function called: 
    domain = zoho.com
    rank = 1121

    Operation done successfully
    Time taken: 59650
    I (783) sqlite3_sdmmc: Card unmounted
    I (783) main_task: Returned from app_main()

## Log

* Last compile: November 03t 2023.
* Last test: November 03t 2023.
* Last compile espidf version: `v 5.1.1`.
* VSCode ESP-IDF Visual Studio Code Extension `v 1.6.5`.

## License

Apache License, Version 2.0, January 2004.

## Version

`v 1.0.0`