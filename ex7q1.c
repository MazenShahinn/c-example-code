#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int x, y;
    int width, height;
    int player;
    int** districts;
} City;

typedef struct {
    int** game_grid;
    City* cities;
    int num_cities;
    int current_player;
    int game_over;
    int winner;
    int grid_width, grid_height;
    int grid_offset_x, grid_offset_y;
    int** salespeople;
} GameState;

typedef int (*CommandHandler)(GameState* game, char** args);

typedef struct {
    char* command_name;
    CommandHandler handler;
} CommandEntry;

int** allocate_2d_array(int rows, int cols) {
    int** array = (int**)malloc(rows * sizeof(int*));
    if (array == NULL) {
        return NULL;
    }
    for (int i = 0; i < rows; i++) {
        array[i] = (int*)malloc(cols * sizeof(int));
        if (array[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(array[j]);
            }
            free(array);
            return NULL;
        }
    }
    return array;
}

void free_2d_array(int** array, int rows) {
    if (array == NULL) return;
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }
    free(array);
}

#define HIT 1
#define MONOPOLIZED 2
#define GAME_OVER 3
#define REPORT 4
#define FAIL 5

void init_game_state(GameState* game) {
    game->game_grid = NULL;
    game->cities = NULL;
    game->num_cities = 0;
    game->current_player = 1;
    game->game_over = 0;
    game->winner = 0;
    game->grid_width = 0;
    game->grid_height = 0;
    game->grid_offset_x = 0;
    game->grid_offset_y = 0;
    game->salespeople = NULL;
}

void add_city(GameState* game, int x, int y, int width, int height, int player) {
    game->cities = realloc(game->cities, sizeof(City) * (game->num_cities + 1));
    
    game->cities[game->num_cities].x = x;
    game->cities[game->num_cities].y = y;
    game->cities[game->num_cities].width = width;
    game->cities[game->num_cities].height = height;
    game->cities[game->num_cities].player = player;
    
    game->cities[game->num_cities].districts = allocate_2d_array(height, width);
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            game->cities[game->num_cities].districts[i][j] = 0;
        }
    }
    
    game->num_cities++;
}

void free_cities(GameState* game) {
    for (int i = 0; i < game->num_cities; i++) {
        free_2d_array(game->cities[i].districts, game->cities[i].height);
    }
    free(game->cities);
    if (game->salespeople) {
        free_2d_array(game->salespeople, game->grid_height);
    }
}

int handle_p1_cities(GameState* game, char** args) {
    printf("P1:\n");
    char line[256];
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    
    while (fgets(line, sizeof line, stdin)) {
        if (strncmp(line, "done", 4) == 0) break;
        
        int x, y, w, h;
        if (sscanf(line, "(%d,%d),%d,%d", &x, &y, &w, &h) == 4) {
            add_city(game, x, y, w, h, 1);
            
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            int rx = x + w;
            int ry = y + h;
            if (rx > maxX) maxX = rx;
            if (ry > maxY) maxY = ry;
        }
    }
    
    game->grid_offset_x = minX;
    game->grid_offset_y = minY;
    game->grid_width = maxX - minX;
    game->grid_height = maxY - minY;
    
    return 1;
}

int handle_p2_cities(GameState* game, char** args) {
    printf("P2:\n");
    char line[256];
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    
    while (fgets(line, sizeof line, stdin)) {
        if (strncmp(line, "done", 4) == 0) break;
        
        int x, y, w, h;
        if (sscanf(line, "(%d,%d),%d,%d", &x, &y, &w, &h) == 4) {
            add_city(game, x, y, w, h, 2);
            
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            int rx = x + w;
            int ry = y + h;
            if (rx > maxX) maxX = rx;
            if (ry > maxY) maxY = ry;
        }
    }
    
    if (minX < game->grid_offset_x) game->grid_offset_x = minX;
    if (minY < game->grid_offset_y) game->grid_offset_y = minY;
    if (maxX > game->grid_offset_x + game->grid_width) 
        game->grid_width = maxX - game->grid_offset_x;
    if (maxY > game->grid_offset_y + game->grid_height) 
        game->grid_height = maxY - game->grid_offset_y;
    
    printf("(%d, %d), %d, %d\n", game->grid_offset_x, game->grid_offset_y, 
           game->grid_width, game->grid_height);
    
    game->game_grid = allocate_2d_array(game->grid_height, game->grid_width);
    game->salespeople = allocate_2d_array(game->grid_height, game->grid_width);
    
    for (int i = 0; i < game->grid_height; i++) {
        for (int j = 0; j < game->grid_width; j++) {
            game->game_grid[i][j] = 0;
            game->salespeople[i][j] = 0;
        }
    }
    
    return 1;
}

int handle_move(GameState* game, char** args) {
    char input[256];
    fgets(input, sizeof(input), stdin);
    
    int x, y;
    if (sscanf(input, "(%d,%d)", &x, &y) == 2) {
        int result = process_salesperson_move(game, x, y);
        
        switch(result) {
            case HIT:
                printf("H\n");
                break;
            case MONOPOLIZED:
                printf("M\n");
                break;
            case GAME_OVER:
                printf("G\n");
                game->game_over = 1;
                break;
            case REPORT:
                {
                    int north, south, east, west;
                    calculate_report_direction(game, x, y, &north, &south, &east, &west);
                    printf("R (%d,%d,%d,%d)\n", north, south, east, west);
                }
                game->current_player = (game->current_player == 1) ? 2 : 1;
                break;
            case FAIL:
                printf("F\n");
                game->current_player = (game->current_player == 1) ? 2 : 1;
                break;
        }
        return 1;
    }
    return 0;
}

int handle_forfeit(GameState* game, char** args) {
    printf("G\n");
    game->game_over = 1;
    game->winner = (game->current_player == 1) ? 2 : 1;
    return 1;
}

int handle_show(GameState* game, char** args) {
    printf("Current player: %d\n", game->current_player);
    printf("Cities: %d\n", game->num_cities);
    for (int i = 0; i < game->num_cities; i++) {
        printf("City %d: Player %d, (%d,%d), %dx%d\n", 
               i, game->cities[i].player, game->cities[i].x, game->cities[i].y, 
               game->cities[i].width, game->cities[i].height);
    }
    return 1;
}

int calculate_heuristic(GameState* game) {
    int diff = 0;
    for (int i = 0; i < game->num_cities; i++) {
        if (game->cities[i].player != game->current_player) {
            for (int cy = 0; cy < game->cities[i].height; cy++) {
                for (int cx = 0; cx < game->cities[i].width; cx++) {
                    if (game->cities[i].districts[cy][cx] == 0) {
                        diff++;
                    }
                }
            }
        }
    }
    return diff;
}

void calculate_report_direction(GameState* game, int x, int y, int* north, int* south, int* east, int* west) {
    int min_distance = INT_MAX;
    int closest_city_x = 0, closest_city_y = 0;
    
    for (int i = 0; i < game->num_cities; i++) {
        if (game->cities[i].player != game->current_player) {
            for (int cy = 0; cy < game->cities[i].height; cy++) {
                for (int cx = 0; cx < game->cities[i].width; cx++) {
                    if (game->cities[i].districts[cy][cx] == 0) {
                        int city_world_x = game->cities[i].x + cx;
                        int city_world_y = game->cities[i].y + cy;
                        int dx = x - city_world_x;
                        int dy = y - city_world_y;
                        int distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
                        
                        if (distance < min_distance) {
                            min_distance = distance;
                            closest_city_x = city_world_x;
                            closest_city_y = city_world_y;
                        }
                    }
                }
            }
        }
    }
    
    int dx = closest_city_x - x;
    int dy = closest_city_y - y;
    
    *north = (dy > 0) ? 1 : 0;
    *south = (dy < 0) ? 1 : 0;
    *east = (dx > 0) ? 1 : 0;
    *west = (dx < 0) ? 1 : 0;
}

int process_salesperson_move(GameState* game, int x, int y) {
    int grid_x = x - game->grid_offset_x;
    int grid_y = y - game->grid_offset_y;
    
    if (grid_x < 0 || grid_x >= game->grid_width || 
        grid_y < 0 || grid_y >= game->grid_height) {
        return FAIL;
    }
    
    if (game->salespeople[grid_y][grid_x] != 0) {
        return FAIL;
    }
    
    for (int i = 0; i < game->num_cities; i++) {
        if (game->cities[i].player == game->current_player) {
            if (x >= game->cities[i].x && x < game->cities[i].x + game->cities[i].width &&
                y >= game->cities[i].y && y < game->cities[i].y + game->cities[i].height) {
                return FAIL;
            }
        }
    }
    
    for (int i = 0; i < game->num_cities; i++) {
        if (game->cities[i].player != game->current_player) {
            if (x >= game->cities[i].x && x < game->cities[i].x + game->cities[i].width &&
                y >= game->cities[i].y && y < game->cities[i].y + game->cities[i].height) {
                
                game->salespeople[grid_y][grid_x] = game->current_player;
                
                int city_x = x - game->cities[i].x;
                int city_y = y - game->cities[i].y;
                game->cities[i].districts[city_y][city_x] = 1;
                
                int fully_monopolized = 1;
                for (int cy = 0; cy < game->cities[i].height; cy++) {
                    for (int cx = 0; cx < game->cities[i].width; cx++) {
                        if (game->cities[i].districts[cy][cx] == 0) {
                            fully_monopolized = 0;
                            break;
                        }
                    }
                    if (!fully_monopolized) break;
                }
                
                if (fully_monopolized) {
                    int enemy_cities_remaining = 0;
                    for (int j = 0; j < game->num_cities; j++) {
                        if (game->cities[j].player != game->current_player) {
                            int city_monopolized = 1;
                            for (int cy = 0; cy < game->cities[j].height; cy++) {
                                for (int cx = 0; cx < game->cities[j].width; cx++) {
                                    if (game->cities[j].districts[cy][cx] == 0) {
                                        city_monopolized = 0;
                                        break;
                                    }
                                }
                                if (!city_monopolized) break;
                            }
                            if (!city_monopolized) {
                                enemy_cities_remaining = 1;
                                break;
                            }
                        }
                    }
                    
                    if (enemy_cities_remaining) {
                        return MONOPOLIZED;
                    } else {
                        game->winner = game->current_player;
                        return GAME_OVER;
                    }
                } else {
                    return HIT;
                }
            }
        }
    }
    
    game->salespeople[grid_y][grid_x] = game->current_player;
    return REPORT;
}

int process_command(GameState* game, char* input) {
    char command[256];
    sscanf(input, "%s", command);
    
    if (strcmp(command, "P1:") == 0) {
        return handle_p1_cities(game, NULL);
    } else if (strcmp(command, "P2:") == 0) {
        return handle_p2_cities(game, NULL);
    } else if (strcmp(command, "f") == 0) {
        return handle_forfeit(game, NULL);
    } else if (strncmp(command, "(", 1) == 0) {
        return handle_move(game, NULL);
    } else if (strcmp(command, "show") == 0) {
        return handle_show(game, NULL);
    }
    
    return 0;
}

void game_loop(GameState* game) {
    char input[256];
    
    while (!game->game_over) {
        printf("P%d:\n", game->current_player);
        fgets(input, sizeof(input), stdin);
        
        if (process_command(game, input)) {
        }
    }
    
    printf("P%d wins\n", game->winner);
}

int main(void) {
    // Initialize game state (following PoE2's __init__ pattern)
    GameState game;
    init_game_state(&game);
    
    // Start the game loop (following PoE2's main_loop pattern)
    game_loop(&game);
    
    // Clean up memory (following PoE2's cleanup pattern)
    if (game.game_grid) {
        free_2d_array(game.game_grid, game.grid_height);
    }
    free_cities(&game);
    
    return 0;
}