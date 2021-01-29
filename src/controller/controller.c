#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "controller.h"
#include "../core/core.h"

#define SELECTION 0
#define ENTRY_TYPE 1
#define ENTRY_NUMBER 2
#define ENTRY_NUMBER_ENTRY 3
#define ENTRY_UNARY 4
#define ENTRY_BINARY 5
#define PERFORM 6
#define EVALUATE 7
#define EVALUATE_ENTRY 8

static char page, selection[8], level, mode;
static const char *template = "+0.000000E+00";
static char buf[14];
static double x, start, end, chunk;

static void render_selection(void) {
    for (int i = 0; i < 10; ++i) {
        mvprintw(i, 0, "%03d", page * 10 + i + 1);
        switch (expression[page * 10 + i].type) {
        case NOP:
            mvprintw(i, 4, "%13s", " ");
            break;
        case NUMBER:
            mvprintw(i, 4,
                     "%+.6E",
                     expression[page * 10 + i].data.number);
            break;
        case UNARY:
            mvprintw(i, 4,
                     "%13s",
                     unary_names[expression[page * 10 + i].data.unary]);
            break;
        case BINARY:
            mvprintw(i, 4,
                     "%13s",
                     binary_names[expression[page * 10 + i].data.binary]);
            break;
        case INPUT:
            mvprintw(i, 4, "%13s", "X");
            break;
        }
    }
    move(selection[0], 16);
}

static void render_entry_type(void) {
    for (int i = 0; i < 5; ++i) {
        mvprintw(11 + i, 0, "%s", type_names[i]);
    }
    move(11 + selection[level], 0);
}

static void remove_entry_type(void) {
    for (int i = 0; i < 5; ++i) {
        mvprintw(11 + i, 0, "%6s", " ");
    }
}

static void render_entry_number(void) {
    static const char *names[2] = {
        "Entry",
        "pi"
    };
    for (int i = 0; i < 2; ++i) {
        mvprintw(11 + i, 7, "%s", names[i]);
    }
    move(11 + selection[level], 7);
}

static void remove_entry_number(void) {
    for (int i = 0; i < 5; ++i) {
        mvprintw(11 + i, 7, "%5s", " ");
    }
}

static void render_entry_unary(void) {
    for (int i = 0; i < 12; ++i) {
        mvprintw(11 + i, 7, "%s", unary_names[i]);
    }
    move(11 + selection[level], 7);
}

static void remove_entry_unary(void) {
    for (int i = 0; i < 12; ++i) {
        mvprintw(11 + i, 7, "%5s", " ");
    }
}

static void render_entry_binary(void) {
    for (int i = 0; i < 5; ++i) {
        mvprintw(11 + i, 7, "%s", binary_names[i]);
    }
    move(11 + selection[level], 7);
}

static void remove_entry_binary(void) {
    for (int i = 0; i < 5; ++i) {
        mvprintw(11 + i, 7, "%s", " ");
    }
}

static void render_perform(void) {
    static const char *names[3] = {
        "Evaluate",
        "Integrate",
        "Plot"
    };
    for (int i = 0; i < 3; ++i) {
        mvprintw(11 + i, 0, "%s", names[i]);
    }
    move(11 + selection[level], 0);
}

static void remove_perform(void) {
    for (int i = 0; i < 3; ++i) {
        mvprintw(11 + i, 0, "%9s", " ");
    }
}

static void render_evaluate(void) {
    mvprintw(11, 10, "%s %+.6E", "X", x);
    mvprintw(12, 10, "Evaluate");
    move(11, 10);
}

static void remove_evaluate(void) {
    for (int i = 0; i < 3; ++i) {
        mvprintw(11 + i, 10, "%15s", " ");
    }
}

void controller_initialize(void) {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    page = 0;
    level = 0;
    memset(selection, 0, 8);
    mode = SELECTION;
    x = 0;
    start = 0;
    end = 0;
    chunk = 0;
    render_selection();
}

void controller_finalize(void) {
    endwin();
}

int controller_handle(void) {
    int in = getch();
    double res;
    int ret;
    switch (in) {
    case 'Q':
    case 'q':
        return 0;
    case KEY_LEFT:
    case 'A':
    case 'a':
        switch (mode) {
        case SELECTION:
            page = (((page - 1) % 10) + 10) % 10;
            render_selection();
            break;
        case ENTRY_NUMBER_ENTRY:
            selection[level] = (((selection[level] - 1) % 13) + 13) % 13;
            move(11, 13 + selection[level]);
            break;
        case EVALUATE_ENTRY:
            selection[level] = (((selection[level] - 1) % 13) + 13) % 13;
            move(11, 12 + selection[level]);
            break;
        }
        break;
    case KEY_RIGHT:
    case 'D':
    case 'd':
        switch (mode) {
        case SELECTION:
            page = (page + 1) % 10;
            render_selection();
            break;
        case ENTRY_NUMBER_ENTRY:
            selection[level] = (selection[level] + 1) % 13;
            move(11, 13 + selection[level]);
            break;
        case EVALUATE_ENTRY:
            selection[level] = (selection[level] + 1) % 13;
            move(11, 12 + selection[level]);
            break;
        }
        break;
    case KEY_UP:
    case 'W':
    case 'w':
        switch (mode) {
        case SELECTION:
            selection[level] = (((selection[level] - 1) % 10) + 10) % 10;
            move(selection[level], 16);
            break;
        case ENTRY_TYPE:
            selection[level] = (((selection[level] - 1) % 5) + 5) % 5;
            move(11 + selection[level], 0);
            break;
        case ENTRY_NUMBER:
            selection[level] = (((selection[level] - 1) % 2) + 2) % 2;
            move(11 + selection[level], 7);
            break;
        case ENTRY_NUMBER_ENTRY:
            switch (selection[level]) {
            case 0:
            case 10:
                if (buf[1] == '0') {
                    break;
                }
                switch (buf[selection[level]]) {
                case '+':
                    buf[selection[level]] = '-';
                    break;
                case '-':
                    buf[selection[level]] = '+';
                    break;
                }
                break;
            case 1:
                if (buf[1] == '9') {
                    buf[1] = '0';
                    strcpy(buf, template);
                } else {
                    ++buf[1];
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                if (buf[1] == '0') {
                    break;
                }
                if (buf[selection[level]] == '9') {
                    buf[selection[level]] = '0';
                } else {
                    ++buf[selection[level]];
                }
                break;
            }
            mvprintw(11, 13, "%s", buf);
            move(11, 13 + selection[level]);
            break;
        case ENTRY_UNARY:
            selection[level] = (((selection[level] - 1) % 12) + 12) % 12;
            move(11 + selection[level], 7);
            break;
        case ENTRY_BINARY:
            selection[level] = (((selection[level] - 1) % 5) + 5) % 5;
            move(11 + selection[level], 7);
            break;
        case PERFORM:
            selection[level] = (((selection[level] - 1) % 3) + 3) % 3;
            move(11 + selection[level], 0);
            break;
        case EVALUATE:
            selection[level] = (((selection[level] - 1) % 2) + 2) % 2;
            move(11 + selection[level], 10);
            break;
        case EVALUATE_ENTRY:
            switch (selection[level]) {
            case 0:
            case 10:
                if (buf[1] == '0') {
                    break;
                }
                switch (buf[selection[level]]) {
                case '+':
                    buf[selection[level]] = '-';
                    break;
                case '-':
                    buf[selection[level]] = '+';
                    break;
                }
                break;
            case 1:
                if (buf[1] == '9') {
                    buf[1] = '0';
                    strcpy(buf, template);
                } else {
                    ++buf[1];
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                if (buf[1] == '0') {
                    break;
                }
                if (buf[selection[level]] == '9') {
                    buf[selection[level]] = '0';
                } else {
                    ++buf[selection[level]];
                }
                break;
            }
            mvprintw(11, 12, "%s", buf);
            move(11, 12 + selection[level]);
            break;
        }
        break;
    case KEY_DOWN:
    case 'S':
    case 's':
        switch (mode) {
        case SELECTION:
            selection[level] = (selection[level] + 1) % 10;
            move(selection[level], 16);
            break;
        case ENTRY_TYPE:
            selection[level] = (selection[level] + 1) % 5;
            move(11 + selection[level], 0);
            break;
        case ENTRY_NUMBER:
            selection[level] = (selection[level] + 1) % 2;
            move(11 + selection[level], 7);
            break;
        case ENTRY_NUMBER_ENTRY:
            switch (selection[level]) {
            case 0:
            case 10:
                if (buf[1] == '0') {
                    break;
                }
                switch (buf[selection[level]]) {
                case '+':
                    buf[selection[level]] = '-';
                    break;
                case '-':
                    buf[selection[level]] = '+';
                    break;
                }
                break;
            case 1:
                switch (buf[1]) {
                case '0':
                    buf[1] = '9';
                    break;
                case '1':
                    --buf[1];
                    strcpy(buf, template);
                    break;
                default:
                    --buf[1];
                    break;
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                if (buf[1] == '0') {
                    break;
                }
                if (buf[selection[level]] == '0') {
                    buf[selection[level]] = '9';
                } else {
                    --buf[selection[level]];
                }
                break;
            }
            mvprintw(11, 13, "%s", buf);
            move(11, 13 + selection[level]);
            break;
        case ENTRY_UNARY:
            selection[level] = (selection[level] + 1) % 12;
            move(11 + selection[level], 7);
            break;
        case ENTRY_BINARY:
            selection[level] = (selection[level] + 1) % 5;
            move(11 + selection[level], 7);
            break;
        case PERFORM:
            selection[level] = (selection[level] + 1) % 3;
            move(11 + selection[level], 0);
            break;
        case EVALUATE:
            selection[level] = (selection[level] + 1) % 2;
            move(11 + selection[level], 10);
            break;
        case EVALUATE_ENTRY:
            switch (selection[level]) {
            case 0:
            case 10:
                mvprintw(10, 0, "%s", buf);
                if (buf[1] == '0') {
                    break;
                }
                switch (buf[selection[level]]) {
                case '+':
                    buf[selection[level]] = '-';
                    break;
                case '-':
                    buf[selection[level]] = '+';
                    break;
                }
                break;
            case 1:
                switch (buf[1]) {
                case '0':
                    buf[1] = '9';
                    break;
                case '1':
                    --buf[1];
                    strcpy(buf, template);
                    break;
                default:
                    --buf[1];
                    break;
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                if (buf[1] == '0') {
                    break;
                }
                if (buf[selection[level]] == '0') {
                    buf[selection[level]] = '9';
                } else {
                    --buf[selection[level]];
                }
                break;
            }
            mvprintw(11, 12, "%s", buf);
            move(11, 12 + selection[level]);
            break;
        }
        break;
    case 'Z':
    case 'z':
    case 'J':
    case 'j':
        switch (mode) {
        case SELECTION:
            mode = ENTRY_TYPE;
            ++level;
            selection[level] = 0;
            render_entry_type();
            break;
        case ENTRY_TYPE:
            switch (selection[level]) {
            case NOP:
                mode = SELECTION;
                selection[level] = 0;
                --level;
                expression[page * 10 + selection[level]].type = NOP;
                remove_entry_type();
                render_selection();
                break;
            case NUMBER:
                mode = ENTRY_NUMBER;
                ++level;
                render_entry_number();
                break;
            case UNARY:
                mode = ENTRY_UNARY;
                ++level;
                render_entry_unary();
                break;
            case BINARY:
                mode = ENTRY_BINARY;
                ++level;
                render_entry_binary();
                break;
            case INPUT:
                mode = SELECTION;
                selection[level] = 0;
                --level;
                expression[page * 10 + selection[level]].type = INPUT;
                remove_entry_type();
                render_selection();
                break;
            }
            break;
        case ENTRY_NUMBER:
            switch (selection[level]) {
            case 0:
                mode = ENTRY_NUMBER_ENTRY;
                ++level;
                strcpy(buf, template);
                mvprintw(11, 13, "%s", template);
                move(11, 13);
                break;
            case 1:
                mode = SELECTION;
                level = 0;
                memset(selection + 1, 0, 7);
                expression[page * 10 + selection[level]].type = NUMBER;
                expression[page * 10 + selection[level]].data.number = pi;
                remove_entry_number();
                remove_entry_type();
                render_selection();
                break;
            }
            break;
        case ENTRY_NUMBER_ENTRY:
            mode = SELECTION;
            level = 0;
            expression[page * 10 + selection[0]].type = NUMBER;
            expression[page * 10 + selection[0]].data.number = atof(buf);
            strcpy(buf, template);
            memset(selection + 1, 0, 7);
            mvprintw(11, 13, "%13s", " ");
            remove_entry_number();
            remove_entry_type();
            render_selection();
            break;
        case ENTRY_UNARY:
            mode = SELECTION;
            level = 0;
            expression[page * 10 + selection[0]].type = UNARY;
            expression[page * 10 + selection[0]].data.unary = selection[2];
            memset(selection + 1, 0, 7);
            remove_entry_unary();
            remove_entry_type();
            render_selection();
            break;
        case ENTRY_BINARY:
            mode = SELECTION;
            level = 0;
            expression[page * 10 + selection[0]].type = BINARY;
            expression[page * 10 + selection[0]].data.unary = selection[2];
            memset(selection + 1, 0, 7);
            remove_entry_binary();
            remove_entry_type();
            render_selection();
            break;
        case PERFORM:
            switch (selection[level]) {
            case 0:
                mode = EVALUATE;
                ++level;
                render_evaluate();
                break;
            }
            break;
        case EVALUATE:
            switch (selection[level]) {
            case 0:
                mode = EVALUATE_ENTRY;
                ++level;
                sprintf(buf, "%+.6E", x);
                move(11, 12);
                break;
            case 1:
                ret = core_evaluate(x, &res);
                if (!ret) {
                    mvprintw(10, 0, "Result: %+.6E", res);
                } else {
                    mvprintw(10, 0, "Result: %13s", "Error");
                }
                getch();
                mvprintw(10, 0, "%21s", " ");
                move(11 + selection[level], 10);
                break;
            }
            break;
        case EVALUATE_ENTRY:
            mode = EVALUATE;
            selection[level] = 0;
            --level;
            x = atof(buf);
            move(11, 10);
            break;
        }
        break;
    case 'X':
    case 'x':
    case 'K':
    case 'k':
        switch (mode) {
        case SELECTION:
            mode = PERFORM;
            ++level;
            render_perform();
            break;
        case ENTRY_TYPE:
            mode = SELECTION;
            selection[level] = 0;
            --level;
            remove_entry_type();
            move(selection[level], 16);
            break;
        case ENTRY_NUMBER:
            mode = ENTRY_TYPE;
            selection[level] = 0;
            --level;
            remove_entry_number();
            move(11 + selection[level], 0);
            break;
        case ENTRY_UNARY:
            mode = ENTRY_TYPE;
            selection[level] = 0;
            --level;
            remove_entry_unary();
            move(11 + selection[level], 0);
            break;
        case ENTRY_BINARY:
            mode = ENTRY_TYPE;
            selection[level] = 0;
            --level;
            remove_entry_binary();
            move(11 + selection[level], 0);
            break;
        case ENTRY_NUMBER_ENTRY:
            mode = ENTRY_NUMBER;
            selection[level] = 0;
            --level;
            strcpy(buf, template);
            mvprintw(11, 13, "%13s", " ");
            move(11, 7);
            break;
        case PERFORM:
            mode = SELECTION;
            selection[level] = 0;
            --level;
            remove_perform();
            move(selection[level], 16);
            break;
        case EVALUATE:
            mode = PERFORM;
            selection[level] = 0;
            --level;
            x = 0;
            remove_evaluate();
            move(11 + selection[level], 0);
            break;
        case EVALUATE_ENTRY:
            mode = EVALUATE;
            selection[level] = 0;
            --level;
            sprintf(buf, "%+.6E", x);
            mvprintw(11, 12, "%+.6E", x);
            move(11, 10);
            break;
        }
        break;
    }
    return 1;
}
