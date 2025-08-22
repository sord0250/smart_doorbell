#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#include "lib/buttons.h"
#include "lib/colors.h"
#include "lib/device.h"
#include "lib/display.h"
#include "lib/fonts/fonts.h"
#include "lib/image.h"
#include "lib/log.h"
#include "lib/camera.h"
#include "lib/client.h"

#define VIEWER_FOLDER "viewer/"
#define MAX_ENTRIES 8
#define MAX_TEXT_SIZE 400
#define MAX_FILE_NAME 100
int state = 0;

// Colors — Feel free to change these to fit your preference
#define BACKGROUND_COLOR WHITE
#define FONT_COLOR BLACK
#define SELECTED_BG_COLOR BYU_BLUE
#define SELECTED_FONT_COLOR BYU_LIGHT_SAND

// Makes sure to deinitialize everything before program close
void intHandler(int dummy) {
    log_info("Exiting...");
    display_exit();
    exit(0);
}

void clearScreen() {
   display_clear(WHITE);
}

/*
 * Takes in a folder, reads the contents of the folder, filtering out any files that do not end with
 * .log or .bmp. This function should check to make sure the folder exists. It fills in the entries
 * array with all of the entries in the folder, up to 8 (MAX_ENTRIES). The function returns the
 * number of entries it put into the entries array.
 */
int get_entries(char *folder, char entries[][MAX_FILE_NAME]) {
    DIR *dp;
    dp = opendir(folder);
    if (dp != NULL) {
        struct dirent *entry;
        uint8_t count = 0;
        while ((entry = readdir(dp)) != NULL) {
            char *name = entry->d_name;
            int len = strlen(name);
            
            if (strncmp(name+len-4, ".log", 4) == 0) {
                memcpy(entries[count], name, strlen(name)+1);
                count++;
            }
            if (strncmp(name+len-4, ".bmp", 4) == 0) {
                memcpy(entries[count], name, strlen(name)+1);
                count++;
            }
        }
        closedir(dp);
        return count;
    }
    return -1;
}

/*
 * Draws the menu of the screen. It uses the entries array to create the menu, with the num_entries
 * specifying how many entries are in the entries array. The selected parameter is the item in the
 * menu that is selected and should be highlighted. Use BACKGROUND_COLOR, FONT_COLOR,
 * SELECTED_BG_COLOR, and SELECTED_FONT_COLOR to help specify the colors of the background, font,
 * select bar color, and selected text color.
 */
void draw_menu(char entries[][MAX_FILE_NAME], int num_entries, int selected) { 
    clearScreen();
    uint8_t y_val = 2;
    for (int i = 0; i < num_entries; i++, y_val += 15) {
        if (i == selected) {
            uint8_t bottom_y = y_val + 13;
            display_draw_rectangle(0, y_val, DISPLAY_WIDTH, bottom_y, BYU_BLUE, 1, 1);
            display_draw_string(2, y_val, entries[i], &Font12, BYU_BLUE, WHITE);
        }
        else {
            display_draw_string(2, y_val, entries[i], &Font12, WHITE, BLACK);
        }
    }
    if (state == 1) {
        display_draw_rectangle(0, DISPLAY_WIDTH - 12, DISPLAY_WIDTH, DISPLAY_WIDTH, BYU_BLUE, 1, 1);
        display_draw_string(2, DISPLAY_WIDTH - 10, "Sending...", &Font12, BYU_BLUE, WHITE);

    }
    if (state == 2) {
        display_draw_rectangle(0, DISPLAY_WIDTH - 12, DISPLAY_WIDTH, DISPLAY_WIDTH, BYU_GREEN, 1, 1);
        display_draw_string(2, DISPLAY_WIDTH - 10, "Sent!", &Font12, BYU_GREEN, WHITE);

    }
}

/*
 * Displays an image or a log file. This function detects the type of file that should be draw. If
 * it is a bmp file, then it calls display_draw_image. If it is a log file, it opens the file, reads
 * 100 characters (MAX_TEXT_SIZE), and displays the text using display_draw_string. Combine folder
 * and file_name to get the complete file path.
 */
void draw_file(char *folder, char *file_name) {
    clearScreen();

    int len = strlen(file_name);
    char file_path[MAX_FILE_NAME];
    sprintf(file_path,"%s%s", folder, file_name);


    if (strncmp(file_name+len-4, ".log", 4) == 0) {

        FILE *fp;
        fp = fopen(file_path, "r");
        char data[MAX_TEXT_SIZE];
        fread(data, 1, MAX_TEXT_SIZE, fp);
        data[MAX_TEXT_SIZE-1] = 0;

        display_draw_string(2, 2, data, &Font12, WHITE, BLACK);
        fclose(fp);
    }

    if (strncmp(file_name+len-4, ".bmp", 4) == 0) {
        display_draw_image(file_path);
    }
}

void *send_image(void *arg) {
    Config cfig = *((Config *) arg);
    int sockfd = client_connect(&cfig);
    client_send_image(sockfd, &cfig);
    client_receive_response(sockfd);
    client_close(sockfd);
    free(cfig.payload);
    state = 2;
    sleep(2);
    state = 3;
    return NULL;
}

int hidden_menu() {
    char folder[] = "viewer/";
    char entries[MAX_ENTRIES][MAX_FILE_NAME];
    int num_entries = get_entries(folder, entries);
    uint8_t selected = 0;
    int o_state = 0;
    draw_menu(entries, num_entries, selected);
    
    while (true) {
            delay_ms(200);

            if (o_state != state) {
                draw_menu(entries, num_entries, selected);
                o_state = state;
            }
            if (state == 3) {
                draw_menu(entries, num_entries, selected);
                state = 0;
            }

            if (button_up() == 0) {
                // Do something upon detecting button press
                if (selected > 0) {
                    --selected;
                } else if (selected == 0) {
                    selected = num_entries-1;
                }
                draw_menu(entries, num_entries, selected);

                while (button_up() == 0) {
                    // Delay while the button is pressed to avoid repeated actions
                    delay_ms(100);
                }
            }

            if (button_left() == 0) {
                clearScreen();
                return 1;
            }

            if (button_down() == 0) {
                // Do something upon detecting button press
                if (selected < num_entries-1) {
                    ++selected;
                } else if (selected == num_entries-1) {
                    selected = 0;
                }
                draw_menu(entries, num_entries, selected);

                while (button_down() == 0) {
                    // Delay while the button is pressed to avoid repeated actions
                    delay_ms(100);
                }
            }

            if (button_right() == 0) {
                draw_file(folder, entries[selected]);
                delay_ms(2000);
                draw_menu(entries, num_entries, selected);

                while (button_down() == 0) {
                    // Delay while the button is pressed to avoid repeated actions
                    delay_ms(100);
                }

            }

            if (button_center() == 0) {
                size_t bufsize = IMG_SIZE;
                uint8_t *my_new_buf = malloc(sizeof(uint8_t) * bufsize);
                clearScreen();
                char str[] = "Your fly     is down";
                display_draw_string(2, 50, str, &Font16, WHITE, BLACK);
                delay_ms(2000);
                camera_capture_data(my_new_buf, bufsize);
                char f_name[] = "viewer/doorbell.bmp";
                camera_save_to_file(my_new_buf, bufsize, f_name);

                // display contents 
                Bitmap* the_map = (Bitmap*)malloc(sizeof(Bitmap));
                create_bmp(the_map, my_new_buf);
                uint32_t width = the_map->img_width;
                int32_t height = the_map->img_height;
                while (true) {
                    delay_ms(200);
                    display_draw_image_data(the_map->pxl_data, width, height);
                    if (button_right() == 0) {
                        reset_pixel_data(the_map);
                        remove_color_channel(0, the_map);
                    }
                    if (button_left() == 0) {
                        reset_pixel_data(the_map);
                        remove_color_channel(2, the_map);
                    }
                    if (button_up() == 0) {
                        reset_pixel_data(the_map);
                        remove_color_channel(1, the_map);
                    }
                    if (button_down() == 0) {
                        reset_pixel_data(the_map);
                        or_filter(the_map);
                    }
                    if (button_center() == 0) {
                        //create config struct
                        Config cfig;
                        cfig.port = "2240";
                        cfig.host = "ecen224.byu.edu";
                        cfig.hw_id = "C187997D9";
                        cfig.payload = my_new_buf;
                        cfig.payload_size = bufsize;

                        //threads
                        state = 1;
                        pthread_t thread1;
                        pthread_create(&thread1, NULL, send_image, (void *) &cfig);

                        // redraw menu and stuff
                        draw_menu(entries, num_entries, selected);
                        destroy_bmp(the_map);
                        break;
                    }
                }
            }

            // Implement other button logic here
        }

    }



int main(void) {
    sleep(15);

    signal(SIGINT, intHandler);

    log_info("Starting...");

    // Use this to fill in with entries from the directory

    // TODO: Initialize the hardware
    display_init();
    buttons_init();

    // TODO: Get directory contents using get_entries function



    // TODO: Draw menu using draw_menu function
    clearScreen();

    int last_push = 0;
    int farther_push = 0;

    while (true) {
        display_draw_string(2, DISPLAY_WIDTH/2, "GET OUT OF MY SWAMP!", &Font16, WHITE, BLACK);
        if (button_center() == 0) {
            size_t bufsize = IMG_SIZE;
            uint8_t *my_new_buf = malloc(sizeof(uint8_t) * bufsize);
            clearScreen();
            char str[] = "DING DONG!";
            display_draw_string(2, 50, str, &Font16, WHITE, BLACK);
            delay_ms(2000);
            clearScreen();
            camera_capture_data(my_new_buf, bufsize);
            char f_name[] = "viewer/doorbell.bmp";
            camera_save_to_file(my_new_buf, bufsize, f_name);


            Bitmap* the_map = (Bitmap*)malloc(sizeof(Bitmap));
            create_bmp(the_map, my_new_buf);

            Config cfig;
            cfig.port = "2240";
            cfig.host = "ecen224.byu.edu";
            cfig.hw_id = "C187997D9";
            cfig.payload = my_new_buf;
            cfig.payload_size = bufsize;

            //threads
            state = 1;
            pthread_t thread1;
            pthread_create(&thread1, NULL, send_image, (void *) &cfig);
            
            // redraw menu and stuff
            destroy_bmp(the_map);
            farther_push = last_push;
            last_push = 0;
            while (button_center() == 0) {
                // Delay while the button is pressed to avoid repeated actions
                delay_ms(100);
            }
        }
        if (button_up()==0) {
            farther_push = last_push;
            last_push = 2;
            while (button_up() == 0) {
                // Delay while the button is pressed to avoid repeated actions
                delay_ms(100);
            }
        }
        if (button_left()==0) {
            farther_push = last_push;
            last_push = 0;
            while (button_left() == 0) {
                // Delay while the button is pressed to avoid repeated actions
                delay_ms(100);
            }
        }
        if (button_right()==0) {
            farther_push = last_push;
            last_push = 0;
            while (button_right() == 0) {
                // Delay while the button is pressed to avoid repeated actions
                delay_ms(100);
            }
        }
        if (button_down()==0) {
            if ((last_push == 2) && (farther_push == 2)) {
                hidden_menu();
            }
            farther_push = last_push;
            last_push = 0;
            while (button_down() == 0) {
                // Delay while the button is pressed to avoid repeated actions
                delay_ms(100);
            }
        }
    }
    return 0;
}
