#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <stdbool.h> 

#define CTRL(c) ((c) & 0x1f)
#define MAX_ROWS 10000
#define MAX_COLS 4096


wchar_t text_buffer[MAX_ROWS][MAX_COLS];
char filename[256] = "isimsiz_flop.txt";
int total_rows = 1;
int offset_y = 0;
int offset_x = 0;
bool save_on_exit = true; 

bool search_mode = false;
wchar_t search_term[100] = {0};
int search_len = 0;
int last_match_y = -1; 
int last_match_x = -1;

void save_file() {
    if (!save_on_exit) return; 

    FILE *f = fopen(filename, "w");
    if (!f) return;
    for (int i = 0; i < total_rows; i++) {
        fprintf(f, "%ls\n", text_buffer[i]);
    }
    fclose(f);
}

void load_file() {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    int r = 0;
    wchar_t line[MAX_COLS];
    while (fgetws(line, MAX_COLS, f) && r < MAX_ROWS) {
        line[wcscspn(line, L"\n")] = L'\0';
        wcscpy(text_buffer[r], line);
        r++;
    }
    total_rows = (r > 0) ? r : 1;
    fclose(f);
}


void find_next(int *cur_y, int *cur_x) {
    if (search_len == 0) return;

    int start_y = *cur_y;
    int start_x = *cur_x + 1; 


    for (int i = start_y; i < total_rows; i++) {
        wchar_t *line = text_buffer[i];
        wchar_t *found = NULL;

        if (i == start_y) {

            if (start_x < (int)wcslen(line))
                found = wcsstr(line + start_x, search_term);
        } else {

            found = wcsstr(line, search_term);
        }

        if (found) {
            *cur_y = i;
            *cur_x = (int)(found - line);
            last_match_y = *cur_y;
            last_match_x = *cur_x;
            return;
        }
    }


    for (int i = 0; i <= start_y; i++) {
        wchar_t *found = wcsstr(text_buffer[i], search_term);
        if (found) {
            *cur_y = i;
            *cur_x = (int)(found - text_buffer[i]);
            last_match_y = *cur_y;
            last_match_x = *cur_x;
            return;
        }
    }
}

void draw_interface(int cur_y, int cur_x) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int edit_h = rows - 2; 
    int edit_w = cols;


    if (cur_y >= offset_y + edit_h) offset_y = cur_y - edit_h + 1;
    if (cur_y < offset_y) offset_y = cur_y;
    if (cur_x >= offset_x + edit_w) offset_x = cur_x - edit_w + 5;
    if (cur_x < offset_x) offset_x = cur_x;


    attron(A_REVERSE | A_BOLD);
    mvhline(0, 0, ' ', cols);
    mvprintw(0, 1, "FLOP - %s %s", filename, save_on_exit ? "" : "[KAYDEDILMEYECEK]");
    attroff(A_REVERSE | A_BOLD);


    for (int i = 0; i < edit_h; i++) {
        int idx = offset_y + i;
        move(i + 1, 0);
        clrtoeol();
        
        if (idx < total_rows) {
            int len = wcslen(text_buffer[idx]);
            if (len > offset_x) {

                if (search_mode && idx == last_match_y && last_match_x >= offset_x) {
                    

                    if (last_match_x > offset_x) {
                        mvaddnwstr(i + 1, 0, &text_buffer[idx][offset_x], last_match_x - offset_x);
                    }
                    

                    attron(COLOR_PAIR(1) | A_BOLD); 
                    mvaddnwstr(i + 1, last_match_x - offset_x, &text_buffer[idx][last_match_x], search_len);
                    attroff(COLOR_PAIR(1) | A_BOLD);


                    if (last_match_x + search_len < len) {
                        mvaddwstr(i + 1, last_match_x - offset_x + search_len, &text_buffer[idx][last_match_x + search_len]);
                    }

                } else {

                    mvaddwstr(i + 1, 0, &text_buffer[idx][offset_x]);
                }
            }
        }
    }


    attron(A_REVERSE);
    mvhline(rows - 1, 0, ' ', cols);

    if (search_mode) {
        mvprintw(rows - 1, 1, "ARA: %ls_  [Enter: Bul | Ctrl+F: Kapat]", search_term);
    } else {
        mvprintw(rows - 1, 1, "[Ctrl+X: Kaydet] [Ctrl+E: Iptal] [Ctrl+P: Sil] [Ctrl+F: Ara] [Y:%d X:%d]", cur_y + 1, cur_x + 1);
    }
    attroff(A_REVERSE);

 
    move(cur_y - offset_y + 1, cur_x - offset_x);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); 
    if (argc > 1) strncpy(filename, argv[1], 255);
    memset(text_buffer, 0, sizeof(text_buffer));
    load_file();

    initscr(); 
    raw(); 
    keypad(stdscr, TRUE); 
    noecho(); 
    set_escdelay(0);
    

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW); 

    wint_t ch;
    int y = 0, x = 0;

    while (true) {
        draw_interface(y, x);
        int res = get_wch(&ch);

        if (res == KEY_CODE_YES) {
    
            if (!search_mode) {
                switch (ch) {
                    case KEY_UP: if (y > 0) { y--; x = (wcslen(text_buffer[y]) < x) ? wcslen(text_buffer[y]) : x; } break;
                    case KEY_DOWN: if (y < total_rows - 1) { y++; x = (wcslen(text_buffer[y]) < x) ? wcslen(text_buffer[y]) : x; } break;
                    case KEY_LEFT: if (x > 0) x--; else if (y > 0) { y--; x = wcslen(text_buffer[y]); } break;
                    case KEY_RIGHT: if (x < (int)wcslen(text_buffer[y])) x++; else if (y < total_rows - 1) { y++; x = 0; } break;
                    case KEY_BACKSPACE:
                        if (x > 0) {
                            wmemmove(&text_buffer[y][x-1], &text_buffer[y][x], wcslen(text_buffer[y]) - x + 1);
                            x--;
                        } else if (y > 0) {
                            int prev_len = wcslen(text_buffer[y-1]);
                            wcscat(text_buffer[y-1], text_buffer[y]);
                            for(int i=y; i<total_rows-1; i++) wcscpy(text_buffer[i], text_buffer[i+1]);
                            memset(text_buffer[total_rows-1], 0, sizeof(wchar_t)*MAX_COLS);
                            total_rows--; y--; x = prev_len;
                        }
                        break;
                }
            } else {
             
                if (ch == KEY_BACKSPACE) { 
                     if (search_len > 0) {
                        search_term[--search_len] = L'\0';
                        last_match_y = -1; 
                    }
                }
            }
        } else {

            if (ch == CTRL('f')) {
                search_mode = !search_mode;
                if (search_mode) {

                    memset(search_term, 0, sizeof(search_term));
                    search_len = 0;
                } else {

                    last_match_y = -1;
                    last_match_x = -1;
                }
                continue;
            }

            if (search_mode) {

                if (ch == '\n' || ch == 13) {

                    find_next(&y, &x);
                } 
                else if (ch == 127 || ch == 8) { 
                    if (search_len > 0) {
                        search_term[--search_len] = L'\0';
                        last_match_y = -1;
                    }
                }
                else if (ch == 27) { 
                    search_mode = false;
                    last_match_y = -1;
                }
                else if (ch >= 32 && search_len < 99) { 
                    search_term[search_len++] = (wchar_t)ch;
                    search_term[search_len] = L'\0';

                }

            } else {

                if (ch == CTRL('x')) {
                    save_on_exit = true;
                    break;
                }


                if (ch == CTRL('e')) {
                    save_on_exit = false;
                    break;
                }

                if (ch == CTRL('p')) {
                    memset(text_buffer, 0, sizeof(text_buffer));
                    total_rows = 1;
                    y = 0;
                    x = 0;
                    offset_y = 0;
                    offset_x = 0;
                    continue;
                }

                if (ch == '\n' || ch == 13) { 
                    if (total_rows < MAX_ROWS) {
                        for(int i = total_rows; i > y + 1; i--) wcscpy(text_buffer[i], text_buffer[i-1]);
                        wcscpy(text_buffer[y+1], &text_buffer[y][x]);
                        text_buffer[y][x] = L'\0';
                        y++; x = 0; total_rows++;
                    }
                } else if (ch == 127 || ch == 8) { 
                    if (x > 0) {
                        wmemmove(&text_buffer[y][x-1], &text_buffer[y][x], wcslen(text_buffer[y]) - x + 1);
                        x--;
                    }
                } else if (ch >= 32) {
                    int len = wcslen(text_buffer[y]);
                    if (len < MAX_COLS - 1) {
                        wmemmove(&text_buffer[y][x + 1], &text_buffer[y][x], len - x + 1);
                        text_buffer[y][x] = (wchar_t)ch;
                        x++;
                    }
                }
            }
        }
    }
    
    save_file(); 
    endwin();
    return 0;
}
