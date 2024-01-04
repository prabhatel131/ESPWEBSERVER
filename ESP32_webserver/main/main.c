// Input to HTML Server for ESP32

#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spi_flash.h"
#include "esp_flash.h"
#include "esp_event.h"
#include "esp_system.h"

static const char *TAG = "ESP32 Server";
const char *filename = "/fatfs/config.json";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
const char *base_path = "/fatfs";
#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t wifi_event_group;

#define EXAMPLE_ESP_WIFI_SSID "ESP32_webserver"
#define EXAMPLE_ESP_WIFI_PASS "esp12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4

typedef struct
{
    char hardwareType[50];
    char *decodedHardware;
    char parameters[50];
    char *decodedparameters;
    char motorCapacity[50];
    char *decodedmotor;
    char vfdType[50];
    char *decodedvfdType;
    char wifiUsername[50];
    char *decodedUsername;
    char wifiPassword[50];
    char *decodedPassword;
    char serverIP[20];
    char *decodedserverIP;
    char firmwareUpgradeEnable[50];
    char *decodedfirmware;
} Configfile;
Configfile cfg;

void readJsonConfig(const char *filename);
void wifi_init_sta();

char *url_decode(const char *input)
{
    size_t len = strlen(input);
    char *output = (char *)malloc(len + 1); // +1 for null terminator
    if (!output)
    {
        // Handle memory allocation failure
        return NULL;
    }

    size_t i, j = 0;
    for (i = 0; i < len; ++i)
    {
        if (input[i] == '%' && i + 2 < len)
        {
            // Valid percent encoding
            int hex1, hex2;
            if (sscanf(input + i + 1, "%2x", &hex1) == 1)
            {
                output[j++] = hex1;
                i += 2;
            }
        }
        else if (input[i] == '+')
        {
            // '+' is commonly used to represent space in URL encoding
            output[j++] = ' ';
        }
        else
        {
            // Copy the character as is
            output[j++] = input[i];
        }
    }

    output[j] = '\0'; // Null-terminate the string
    return output;
}

esp_err_t get_handler(httpd_req_t *req)
{
    const char resp[] = "<!DOCTYPE HTML><html><head>\
                            <title>ESP Input Form</title>\
                            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
                            </head><body>\
                            <h1>HMI Webserver</h1>\
                            <form action=\"/get\">\
                                Hardware Type: <input type=\"text\" name=\"hardware_type\"><br>\
                                Parameters: <input type=\"text\" name=\"parameters\"><br>\
                                Motor Capacity: <input type=\"text\" name=\"motor_capacity\"><br>\
                                VFD Type: <input type=\"text\" name=\"vfd_type\"><br>\
                                Wifi Username: <input type=\"text\" name=\"wifi_username\"><br>\
                                Wifi Password: <input type=\"text\" name=\"wifi_password\"><br>\
                                Server IP: <input type=\"text\" name=\"server_ip\"><br>\
                                Firmware Upgrade: <input type=\"text\" name=\"firmware_enable\"><br>\
                                <input type=\"submit\" value=\"Submit\">\
                            </form><br>\
                            </body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_handler_str(httpd_req_t *req)
{
    // Read the URI line and get the host
    char *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Host: %s", buf);
        }
        free(buf);
    }

    // Read the URI line and get the parameters
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            // cfg= (Config*)buf;
            ESP_LOGI(TAG, "Found URL query: %s", buf);
            // printf("buffer data :%s\n",cfg.hardwareType);
            if (httpd_query_key_value(buf, "hardware_type", cfg.hardwareType, sizeof(cfg.hardwareType)) == ESP_OK)
            {
                 cfg.decodedHardware = url_decode(cfg.hardwareType);
                if (cfg.decodedHardware)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedHardware);
                    strncpy(cfg.hardwareType, cfg.decodedHardware, sizeof(cfg.hardwareType));
                    free(cfg.decodedHardware);
                }
            }
            if (httpd_query_key_value(buf, "parameters", cfg.parameters, sizeof(cfg.parameters)) == ESP_OK)
            {
                 cfg.decodedparameters = url_decode(cfg.parameters);
                if (cfg.decodedparameters)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedparameters);
                    strncpy(cfg.parameters, cfg.decodedparameters, sizeof(cfg.parameters));
                    free(cfg.decodedparameters);
                }
            }
            if (httpd_query_key_value(buf, "motor_capacity", cfg.motorCapacity, sizeof(cfg.motorCapacity)) == ESP_OK)
            {
               cfg.decodedmotor = url_decode(cfg.motorCapacity);
                if (cfg.decodedmotor)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedmotor);
                    strncpy(cfg.motorCapacity, cfg.decodedmotor, sizeof(cfg.motorCapacity));
                    free(cfg.decodedmotor);
                }
            }
            if (httpd_query_key_value(buf, "vfd_type", cfg.vfdType, sizeof(cfg.vfdType)) == ESP_OK)
            {
                cfg.decodedvfdType = url_decode(cfg.vfdType);
                if (cfg.decodedvfdType)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedvfdType);
                    strncpy(cfg.vfdType, cfg.decodedvfdType, sizeof(cfg.vfdType));
                    free(cfg.decodedvfdType);
                }
            }
            if (httpd_query_key_value(buf, "wifi_username", cfg.wifiUsername, sizeof(cfg.wifiUsername)) == ESP_OK)
            {
               cfg.decodedUsername = url_decode(cfg.wifiUsername);
                if (cfg.decodedvfdType)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedUsername);
                    strncpy(cfg.wifiUsername, cfg.decodedUsername, sizeof(cfg.wifiUsername));
                    free(cfg.decodedUsername);
                }
            }
            if (httpd_query_key_value(buf, "wifi_password", cfg.wifiPassword, sizeof(cfg.wifiPassword)) == ESP_OK)
            {
                // Decode the URL-encoded password
                cfg.decodedPassword = url_decode(cfg.wifiPassword);

                if (cfg.decodedPassword)
                {
                    ESP_LOGI(TAG, "Decoded Wifi Password = %s", cfg.decodedPassword);

                    // Use the decoded password in your logic as needed
                    // For example, store it in your config structure
                    strncpy(cfg.wifiPassword, cfg.decodedPassword, sizeof(cfg.wifiPassword));

                    // Free the memory allocated by url_decode
                    free(cfg.decodedPassword);
                }
            }
            if (httpd_query_key_value(buf, "server_ip", cfg.serverIP, sizeof(cfg.serverIP)) == ESP_OK)
            {
               cfg.decodedserverIP = url_decode(cfg.serverIP);
                if (cfg.decodedserverIP)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedserverIP);
                    strncpy(cfg.serverIP, cfg.decodedserverIP, sizeof(cfg.serverIP));
                    free(cfg.decodedserverIP);
                }
            }
            if (httpd_query_key_value(buf, "firmware_enable", cfg.firmwareUpgradeEnable, sizeof(cfg.firmwareUpgradeEnable)) == ESP_OK)
            {
               cfg.decodedfirmware = url_decode(cfg.firmwareUpgradeEnable);
                if (cfg.decodedfirmware)
                {
                    ESP_LOGI(TAG, "HardwareType = %s", cfg.decodedfirmware);
                    strncpy(cfg.firmwareUpgradeEnable, cfg.decodedfirmware, sizeof(cfg.firmwareUpgradeEnable));
                    free(cfg.decodedfirmware);
                }
            }
        }
        free(buf);
    }

    // The response
    const char resp[] = "The data was sent ...";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    // return ESP_OK;

    // Create a cJSON object for the received data
    cfg.decodedPassword = url_decode(cfg.wifiPassword);
    // printf("password : %s\n",cfg.decodedPassword);

    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddStringToObject(json_data, "hardwareType", cfg.hardwareType);
    cJSON_AddStringToObject(json_data, "parameters", cfg.parameters);
    cJSON_AddStringToObject(json_data, "motorCapacity", cfg.motorCapacity);
    cJSON_AddStringToObject(json_data, "vfdType", cfg.vfdType);
    cJSON_AddStringToObject(json_data, "wifiUsername", cfg.wifiUsername);
    cJSON_AddStringToObject(json_data, "wifiPassword", cfg.decodedPassword);
    cJSON_AddStringToObject(json_data, "serverIP", cfg.serverIP);
    cJSON_AddStringToObject(json_data, "firmwareUpgradeEnable", cfg.firmwareUpgradeEnable);

    printf("password : %s\n", cfg.decodedPassword);

    // // Convert cJSON object to JSON string
    char *json_str = cJSON_Print(json_data);
    // printf("hardwaretype:%s\n", hardwareType);

    // Write the JSON string to a file
    FILE *file = fopen("/fatfs/config.json", "w");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Error opening file for writing: /fatfs/config.json");
        cJSON_Delete(json_data);
        free(json_str);
        return ESP_FAIL;
    }

    fprintf(file, "%s", json_str);

    ESP_LOGI(TAG, "Received data written to /fatfs/config.json");

    cJSON_Delete(json_data);
    fclose(file);
    free(json_str);

    // printf("password : %s\n",decodedPassword);

    readJsonConfig(filename);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    wifi_init_sta();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n\n");

    return ESP_OK;
}

void readJsonConfig(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Error opening file for reading: %s", filename);
        ESP_LOGE(TAG, "Error opening file: %s", strerror(errno));
        return;
    }

    // Read the entire file into a buffer
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    //  Read the entire file into a buffer
    char *json_str = (char *)malloc(file_size + 1);
    if (json_str == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation error");
        fclose(file);
        return;
    }

    fread(json_str, 1, file_size, file);
    json_str[file_size] = '\0'; // Null-terminate the string

    fclose(file);
    ESP_LOGI(TAG, "JSON File Content:\n%s", json_str);

    cJSON *root = cJSON_Parse(json_str);
    free(json_str);

    if (root == NULL)
    {
        ESP_LOGE(TAG, "Error parsing JSON file: %s", filename);
        free(json_str);
        return;
    }

    // Retrieve values from JSON object
    const char *hardwareType = cJSON_GetObjectItemCaseSensitive(root, "hardwareType")->valuestring;
    const char *parameters = cJSON_GetObjectItemCaseSensitive(root, "parameters")->valuestring;
    const char *motorCapacity = cJSON_GetObjectItemCaseSensitive(root, "motorCapacity")->valuestring;
    const char *vfdType = cJSON_GetObjectItemCaseSensitive(root, "vfdType")->valuestring;
    const char *wifiUsername = cJSON_GetObjectItemCaseSensitive(root, "wifiUsername")->valuestring;
    const char *wifiPassword = cJSON_GetObjectItemCaseSensitive(root, "wifiPassword")->valuestring;
    const char *serverIP = cJSON_GetObjectItemCaseSensitive(root, "serverIP")->valuestring;
    const char *firmwareUpgradeEnable = cJSON_GetObjectItemCaseSensitive(root, "firmwareUpgradeEnable")->valuestring;

    // // Print retrieved values

    cJSON_Delete(root);
    // free(json_str);

    ESP_LOGI(TAG, "Reading from file: %s", filename);
    // printf("password : %s\n",decodedPassword);
    // fclose(file);
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_get_input = {
    .uri = "/get",
    .method = HTTP_GET,
    .handler = get_handler_str,
    .user_ctx = NULL};

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_input);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        httpd_stop(server);
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //esp_event_loop_create_default();  
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


static esp_err_t wifi_event_handler_sta(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WiFi connected");
        printf("WiFi CONNECTED\n");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WiFi disconnected");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

        // Attempt to reconnect
        esp_wifi_connect();
        break;

    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;

    default:
        break;
    }

    return ESP_OK;
}

void wifi_init_sta()
{
    esp_netif_init();
    esp_event_loop_create_default();     // event loop
    esp_netif_create_default_wifi_sta(); // WiFi station
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //

    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler_sta, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler_sta, NULL);

    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler_sta, NULL));

    wifi_init_config_t cgf = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cgf));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };

    if (strlen(cfg.wifiUsername) == 0 || strlen(cfg.wifiPassword) == 0)
    {
        ESP_LOGI(TAG, "WiFi credentials not found. Retrieving from server...");

        // Save the updated credentials to the JSON file
        // writeJsonConfig(filename, config);
    }
    cfg.decodedPassword = url_decode(cfg.wifiPassword);
    cfg.decodedHardware = url_decode(cfg.hardwareType);
    strncpy((char *)wifi_config.sta.ssid, cfg.decodedUsername, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, cfg.decodedPassword, sizeof(wifi_config.sta.password) - 1);

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);

    printf("password : %s\n", cfg.decodedPassword);
    // printf("password : %s\n",wifiPassword);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for WiFi to connect...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

void app_main(void)
{

    ESP_LOGI(TAG, "Initializing FATFS");
    // Mount FATFS
    esp_vfs_fat_mount_config_t configuration = {
        .max_files = 1,
        .format_if_mount_failed = true,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};

    esp_err_t err = esp_vfs_fat_spiflash_mount("/fatfs", "storage", &configuration, &s_wl_handle);

    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to mount FATFS file system\n");
        return;
    }

    static httpd_handle_t server = NULL;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the default event loop
     //ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());
    start_webserver();
}