#pragma once
#ifndef __MQTT_H_
#define __MQTT_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include <freertos/event_groups.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"

#define TAG_MQTT "MQTT"

/* @brief indicate that the ESP32 is currently connected. */
const int MQTT_CONNECTED_BIT = BIT0;
static const char mqtt_Client_cert[] = R"(-----BEGIN CERTIFICATE-----
MIIDxzCCAq+gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix
FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE
CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y
ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMjExMjkwMTM2
MjRaFw0yMzAyMjcwMTM2MjRaMIGgMQswCQYDVQQGEwJCUjEOMAwGA1UECAwFQkFI
SUExETAPBgNVBAcMCFNBTFZBRE9SMREwDwYDVQQKDAhQUk9BVElWQTEOMAwGA1UE
CwwFQVVGQVMxJTAjBgNVBAMMHGVzcDMyLnByb2F0aXZhY29uc3VsdG9yaWEuaW8x
JDAiBgkqhkiG9w0BCQEWFXJmYWJyaWNpb3JzQGdtYWlsLmNvbTCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBANd55Okwu2/nEU43QfYxNSQpyi7LcM/POP7c
tlI1et9PmgBJ5bhjsQWbOrrdYW7fwW7yld/58o3FBRMtYadjhuQmO7ZkTwbz/dol
A8prAotC0UsLlsjitKbBhZRf+ya6LUn3Ao+9G05lE46auNPCNqI6yHQhyB2NQqwT
U6OKbanWIBSabABK5cSb2UTTKSXtgJYL1cWxzUDt7ab8H9ZM8C2vRv2lxRlQkhKb
ehuGXI1dAy4Ei/6+FCi+a2sJl5O2cCy3DTMah7ESFHnISOz2XK7YqCN8lO4s9EbQ
pC0A+w/Ev1msrdPdBIf9FcjdtET+2aMJcly24wG2a49r5zs2sjsCAwEAAaMaMBgw
CQYDVR0TBAIwADALBgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADggEBADkQpVlY
8MbBrCt2RtnbpMbNJXDDYiDJSNa3LI2i696sfHznhsGeU/Jp2d3F01r1jIt5+rEh
m3Zy7su1OFlGk6VjLbnkjQwGkNhGeG0vJizD8/rlmh5lP7vCsCxiaVjMnLtXml8l
XUwovjWS7wxsMfADtYpYojR8KYLHIokDfXFXOmNLuLU1Da426jG1NGW7YlsX3iud
UH1YFGgeyj6wlopjN1Van6KEnQpXZIlq05h/wmSW2EGUI76ZXFxG7LS/B5D+wvMc
8cKWApQLQuCluxxSmLYqhdXGJvYISQx/2ZWT48q5tlnNYJPxVjchW+y6yrRI7O5O
5kGlfjmeWUmDHLg=
-----END CERTIFICATE-----
)";
static const char mqtt_Client_key[] = R"(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDXeeTpMLtv5xFO
N0H2MTUkKcouy3DPzzj+3LZSNXrfT5oASeW4Y7EFmzq63WFu38Fu8pXf+fKNxQUT
LWGnY4bkJju2ZE8G8/3aJQPKawKLQtFLC5bI4rSmwYWUX/smui1J9wKPvRtOZROO
mrjTwjaiOsh0IcgdjUKsE1Ojim2p1iAUmmwASuXEm9lE0ykl7YCWC9XFsc1A7e2m
/B/WTPAtr0b9pcUZUJISm3obhlyNXQMuBIv+vhQovmtrCZeTtnAstw0zGoexEhR5
yEjs9lyu2KgjfJTuLPRG0KQtAPsPxL9ZrK3T3QSH/RXI3bRE/tmjCXJctuMBtmuP
a+c7NrI7AgMBAAECggEARX2OqPOp5z9WUeq5L5wT/sspZKkAg0xZnbvxYpbTvxPt
Xv12A97Gtr+mk6AUrYyk1dKPKjHyGu14owQmM7UByi9YX4x+/gePDRyV2eBj7CSK
2dBtevC+AO91VHIVzZoKRAnQvilnFoO7aGiWhdhjBTO+9ND2OC2X3GXJOftJPZV0
oe+Ye9vCsu9mW0+uEd/1QrXlNKNzunLOIsRebD9MKqoWv+8uHhmthCYqJ4If6jDH
r2o8xFAjys2Tdd1oL/tcTeE47CrXHrmAkGX8/meZFkK7hIbAbqdnmPasi4zOuqCb
NjmF5m7gDxrjOtcDvFtaiQv9CGk/WEhPsx4fir6TZQKBgQDzb5y0Ypi8R3zrrWkD
U9NDy0K+R6G30vVYOhikEBvAsXm7LbVROhvCACe6HzUMzvpXV/agoLoiuVPEAZEI
ZnULsu/ZGZuABGsiMCNrowRVXBE1Ps9FFEHiMriaWRoYaVLmBL+7q2cxXSib4lKE
WHzxT0z6W1vRQZ8qCnJUxINMpQKBgQDimN0nfGiH0r3bPR3ofULqQbh393gCUUnk
h7Fwat/psDp+vJpQsb7DwYV/M2Ug0RWh3xQ4vwe6GD7ua9QRN01R1Piz8k25KVvv
+ChgESAanrgwoTIsKEl1ethy1sAFA3zOdoW2CgAfm8D6MpT3jHGLira4czt+1vHl
TAyTDX9tXwKBgCpBhYIKW1jTT+vY/hGFy3ZJIhzFclzmUZAvBXf1E9wu8mJ7XK3s
2ESkokC8igjfJLVkn3ria2z7nn63huVeY5LUTK3ys/tUyQP2Ny/H7Eik4pfLf08O
ZSFOjJy+OrliDvG+gUmiRxdahGIKtygh9A4vJb2PfjCko2w4w//S3LfBAoGBALaZ
kL3dFy88uBoQKiXLqzyzMUAbf+3+K5mpb2ez+DSM+tZejcwxMBT3weg7Nmc3lkMX
TWhdKJDiMPRYmFA3TGuBP9lijReFKL08MO0ZX9GTzcICZDgJTwkIsZv5Sku//7cw
85c6f9b44I2+bST/XBCi+5Ov6GXSOKQaoJdv7RvpAoGAHfgMaewGdOF44ENQDh+h
XbHNA8H5dGSJOYpgcY0oTgvcVtQdpiZW7RLrBJKnQokaJgwN/7MAa7NTZ3mrtadb
F2k3BMpcj99PedKeQ79w5aZWeHeJ+3Psx0FX44CgYD7WQru4htUwgez+qiehH/ah
1KNog+oIidMuDoOtEXEoTNw=
-----END PRIVATE KEY-----
)";
static const char mqtt_CAcert[] = R"(-----BEGIN CERTIFICATE-----
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=
-----END CERTIFICATE-----
)";
extern EventGroupHandle_t mqtt_event_group;
extern esp_mqtt_client_handle_t client;
extern char *mqtt_uri;
extern char mqtt_uri2[128];
extern char *mqtt_subscribe_topic;
extern char *mqtt_publish_topic;


void Task_MQTT_parse(void *pvParm);
void mqtt_app_start(void);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
void set_mqtt_uri(char *);
const char *get_mqtt_uri(void);
const char *get_mqtt_publish(void);
const char *get_mqtt_subscribe(void);

#endif