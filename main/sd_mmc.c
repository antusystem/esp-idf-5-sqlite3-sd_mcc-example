/* ESP-IDF 5 SD_MMC Example
  *
  * This example opens Sqlite3 databases from SD Card and retrieves data from them.
  * Before running please copy following files to SD Card:
  * data/mdr512.db
  * data/chinook.db
  * 
  * I could not open with ESP32 the file census2000names.db from the original example, so I search a SQLite3 sample.
  * 
  * This is adaptation from siara-cc examples (https://github.com/siara-cc/esp32-idf-sqlite3-examples)
  * for it to work on ESP-IDF v5.X.X
*/
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sqlite3.h"
#include "sdmmc_cmd.h"

static const char *TAG = "sqlite3_sdmmc";
const char* data = "Callback function called";
sdmmc_card_t* card;
sqlite3 *db1;
sqlite3 *db2;
int rc;
char *zErrMsg = 0;

/**
  * @brief  SQLite Callback Function
  * 
  * This function is a callback used with SQLite queries to process the results. It prints the
  * values of the result set columns, including their names and data, and an optional custom message.
  * 
  * @param data - A pointer to optional custom data (can be NULL).
  * @param argc - The number of columns in the result set.
  * @param argv - An array of strings containing the values of the result set's columns.
  * @param azColName - An array of strings containing the names of the columns in the result set.
  *
  * @return
  * - 0 to indicate success.
  *
  * @note
  * - The `data` parameter can be used to pass an optional custom message or data.
  * - This function is typically used as a callback for SQLite query execution.
*/
static int callback(void *data, int argc, char **argv, char **azColName) {
    int i;

    // Print the custom message
    printf("%s: \n", (const char*)data);

    // Loop through the result set columns and print their names and values
    for (i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    // Print a newline character to separate rows
    printf("\n");
    return 0;
}

/**
 * @brief Open a SQLite database.
 *  
 * This function opens a SQLite database specified by the filename and provides an SQLite
 * database connection object for further database operations.
 *  
 * @param filename - The name of the database file to open.
 * @param db - A pointer to a pointer to an SQLite database connection object. Upon success,
 *             this pointer will store the reference to the opened database.
 *  
 * @return
 *  - 0 on success, indicating the database was opened successfully.
 *  - A non-zero error code if there was an issue opening the database.
 *
 * @note
 * It is the responsibility of the caller to handle errors appropriately based on the
 * return value.
 *  
 * @see sqlite3_open
 */
int openDb(const char *filename, sqlite3 **db) {
    int rc = sqlite3_open(filename, db);
    if (rc) {
        printf("Can't open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    } else {
        printf("Opened database successfully\n");
    }
    return rc;
}

/**
 * @brief Execute an SQL statement on an SQLite database.
 *
 * This function executes the provided SQL statement on the specified SQLite database
 * and optionally calls a callback function to process the results. It also measures
 * the execution time and provides error handling.
 *
 * @param db - A pointer to the SQLite database connection.
 * @param sql - The SQL statement to be executed.
 *
 * @return
 *  - SQLITE_OK (0) on success, indicating the operation was completed successfully.
 *  - An SQLite error code on failure.
 *
 * @note
 * - The function provides timing information to measure the execution time of the SQL statement.
 * - Error handling is performed, and any SQL errors are printed along with timing information.
 * - The provided `sql` parameter should be a well-formed SQL statement.
 * - The `callback` function, if specified, processes the results of the SQL query.
 */
int db_exec(sqlite3 *db, const char *sql) {
    printf("%s\n", sql);
    int64_t start = esp_timer_get_time();
    int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
    if (rc != SQLITE_OK) {
        // Print SQL error message
        printf("SQL error: %s\n", zErrMsg);
        // Free the error message string
        sqlite3_free(zErrMsg);
    } else {
        printf("Operation done successfully\n");
    }
    // Print execution time
    printf("Time taken: %lld\n", esp_timer_get_time()-start);
    return rc;
}

/**
 * @brief Initialize and Mount SD Card
 *
 * This function initializes and mounts an SD card using the SDMMC peripheral.
 * It configures the SDMMC host, slot, and options for mounting the filesystem.
 * Debug information is logged using ESP-IDF's logging facilities.
 *
 * @return
 * - ESP_OK on success, indicating the SD card was initialized and mounted.
 * - ESP_FAIL on failure, indicating an issue with initialization or mounting.
 *
 * @note
 * - The function configures the SDMMC host and slot, and sets options for mounting the filesystem.
 * - If the mounting fails, it can be due to a lack of pull-up resistors on the SD card lines.
 * - Check for proper hardware setup before calling this function.
 * - If you want the card to be formatted when mounting fails, set `format_if_mount_failed` to true.
 * - Additional error recovery may be needed when developing production applications.
 */
esp_err_t init_sdcard(void){
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // slot_config.width = 1;

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

     // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    // sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        // return;
        return ESP_FAIL;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

/**
 * @brief Select Data from both Databases
 *
 * This function performs SQL SELECT operations to retrieve data from two separate SQLite
 * databases. It handles any errors that may occur during the retrieval process.
 *
 * @note
 * - If an error occurs during data retrieval, both database connections are closed, and
 *   the function returns without completing the second SELECT operation.
 */
void select_data(){
    rc = db_exec(db1, "Select * from albums where AlbumId < '10'");
    if (rc != SQLITE_OK) {
        sqlite3_close(db1);
        sqlite3_close(db2);
        return;
    }

    rc = db_exec(db2, "Select * from domain_rank where domain = 'zoho.com'");
    if (rc != SQLITE_OK) {
        sqlite3_close(db1);
        sqlite3_close(db2);
        return;
    }
}

void app_main(){

    ESP_LOGI(TAG, "Initializing SD card");
    ESP_ERROR_CHECK(init_sdcard());

    sqlite3_initialize();

    // Open database 1
    // if (openDb("/sdcard/census2000names.db", &db1))
    //     return;
    ESP_LOGI(TAG, "Opening db chinook");
    if (openDb("/sdcard/chinook.db", &db1))
        return;
    // Open database 2
    ESP_LOGI(TAG, "Opening db mdr512");
    if (openDb("/sdcard/mdr512.db", &db2))
        return;

    // Selecting data
    select_data();

    // Close SQLite databases.
    sqlite3_close(db1);
    sqlite3_close(db2);

    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount("/sdcard", card);
    ESP_LOGI(TAG, "Card unmounted");
   //while(1);
}
