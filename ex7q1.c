#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// City struct for representing a city in the game
typedef struct {
    int x, y;        // southwest corner coordinates
    int width, height;
    int player;      // 1 or 2
    int** districts; // 2D array to track occupied districts
} City;

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

// Function to add a city to the cities array
void add_city(City** cities, int* num_cities, int* max_cities, 
              int x, int y, int width, int height, int player) {
    // If we need more space, reallocate the array
    if (*num_cities >= *max_cities) {
        *max_cities *= 2;
        *cities = realloc(*cities, sizeof(City) * (*max_cities));
    }
    
    // Add the new city
    (*cities)[*num_cities].x = x;
    (*cities)[*num_cities].y = y;
    (*cities)[*num_cities].width = width;
    (*cities)[*num_cities].height = height;
    (*cities)[*num_cities].player = player;
    
    // Allocate 2D array for districts (0 = empty, 1 = occupied)
    (*cities)[*num_cities].districts = allocate_2d_array(height, width);
    
    // Initialize all districts as empty (0)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            (*cities)[*num_cities].districts[i][j] = 0;
        }
    }
    
    (*num_cities)++;
}

// Function to free all cities and their districts
void free_cities(City* cities, int num_cities) {
    for (int i = 0; i < num_cities; i++) {
        free_2d_array(cities[i].districts, cities[i].height);
    }
    free(cities);
}

static void read_player_and_update_bounds(const char* prompt,
                                          int* minX, int* minY,
                                          int* maxX, int* maxY,
                                          City** cities, int* num_cities, 
                                          int* max_cities, int player) {
    printf("%s:\n", prompt);
    char line[256];
    while (fgets(line, sizeof line, stdin)) {
        if (strncmp(line, "done", 4) == 0) break;

        int x, y, w, h;
        if (sscanf(line, "(%d,%d),%d,%d", &x, &y, &w, &h) == 4) {
            // Add city to the array
            add_city(cities, num_cities, max_cities, x, y, w, h, player);
            
            // Update bounding box
            if (x < *minX) *minX = x;
            if (y < *minY) *minY = y;
            int rx = x + w;  
            int ry = y + h;
            if (rx > *maxX) *maxX = rx;
            if (ry > *maxY) *maxY = ry;
        }
    }
}

int main(void) {
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    
    // Initialize cities array (following the example pattern exactly)
    int num_cities = 0;
    int max_cities = 10; // Start with capacity for 10 cities
    City *cities = malloc(sizeof(City) * max_cities);
    
    if (cities == NULL) {
        printf("Error: Failed to allocate memory for cities array\n");
        return 1;
    }

    // Read player cities and update bounds
    read_player_and_update_bounds("P1", &minX, &minY, &maxX, &maxY, 
                                 &cities, &num_cities, &max_cities, 1);
    read_player_and_update_bounds("P2", &minX, &minY, &maxX, &maxY, 
                                 &cities, &num_cities, &max_cities, 2);

    int width  = maxX - minX;
    int height = maxY - minY;
    printf("(%d, %d), %d, %d\n", minX, minY, width, height);

    // Allocate the 2D array and store the pointer
    int** game_grid = allocate_2d_array(height, width);
    
    // Check if allocation was successful
    if (game_grid == NULL) {
        printf("Error: Failed to allocate memory for game grid\n");
        free_cities(cities, num_cities);
        return 1;
    }
    
    // Print cities for debugging (you can remove this later)
    printf("\nCities loaded:\n");
    for (int i = 0; i < num_cities; i++) {
        printf("City %d: Player %d, (%d,%d), %dx%d\n", 
               i, cities[i].player, cities[i].x, cities[i].y, 
               cities[i].width, cities[i].height);
    }
    
    // TODO: Add game logic here
    
    // Clean up memory before exiting (following the example pattern exactly)
    free_2d_array(game_grid, height);
    free_cities(cities, num_cities);
   
    return 0;
}