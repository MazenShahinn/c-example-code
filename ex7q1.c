#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

static void read_player_and_update_bounds(const char* prompt,
                                          int* minX, int* minY,
                                          int* maxX, int* maxY) {
    printf("%s:\n", prompt);
    char line[256];
    while (fgets(line, sizeof line, stdin)) {
        if (strncmp(line, "done", 4) == 0) break;

        int x, y, w, h;
        if (sscanf(line, "(%d,%d),%d,%d", &x, &y, &w, &h) == 4) {
            if (x < *minX) *minX = x;
            if (y < *minY) *minY = y;
            int rx = x + w;  
            int ry = y + h;  // Fixed: completed the comment
            if (rx > *maxX) *maxX = rx;
            if (ry > *maxY) *maxY = ry;
        }
    }
}

int main(void) {
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    read_player_and_update_bounds("P1", &minX, &minY, &maxX, &maxY);
    read_player_and_update_bounds("P2", &minX, &minY, &maxX, &maxY);

    int width  = maxX - minX;
    int height = maxY - minY;
    printf("(%d, %d), %d, %d\n", minX, minY, width, height);

    // Allocate the 2D array and store the pointer
    int** game_grid = allocate_2d_array(height, width);
    
    // Check if allocation was successful
    if (game_grid == NULL) {
        printf("Error: Failed to allocate memory for game grid\n");
        return 1;
    }
    
    // TODO: Add game logic here
    
    // Clean up memory before exiting
    free_2d_array(game_grid, height);
   
    return 0;
}