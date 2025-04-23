#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>  // Thêm thu vi?n SDL_mixer cho âm thanh
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    Wall(int startX, int startY) {
        x = startX;
        y = startY;
        active = true;
        rect = { x, y, TILE_SIZE, TILE_SIZE };
    }
    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class Bullet {
public:
    int x, y, dx, dy;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Texture* tex) {
        x = startX;
        y = startY;
        dx = dirX;
        dy = dirY;
        texture = tex;
        active = true;
        rect = { x, y, 15, 15 };
    }

    void move() {
        x += dx * 5;
        y += dy * 5;
        rect.x = x;
        rect.y = y;
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE ||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class PlayerTank {
public:
    int x, y, dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    vector<Bullet> bullets;

    PlayerTank() {}
    PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* bTex) {
        x = startX;
        y = startY;
        bulletTexture = bTex;
        rect = { x, y, TILE_SIZE, TILE_SIZE };
        dirX = 0;
        dirY = -1;
        SDL_Surface* surface = IMG_Load("anh111.png");
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void shoot() {
        bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture));
    }

    void updateBullets() {
        for (auto& b : bullets) b.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
        double angle = 0.0;
        if (dirX == 0 && dirY == -1) angle = 0.0;
        else if (dirX == 1 && dirY == 0) angle = 90.0;
        else if (dirX == 0 && dirY == 1) angle = 180.0;
        else if (dirX == -1 && dirY == 0) angle = 270.0;
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);

        for (auto& bullet : bullets) bullet.render(renderer);
    }

    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx, newY = y + dy;
        dirX = (dx != 0) ? dx / abs(dx) : 0;
        dirY = (dy != 0) ? dy / abs(dy) : 0;
        SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE };
        for (const auto& wall : walls)
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) return;

        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE) {
            x = newX; y = newY;
            rect.x = x; rect.y = y;
        }
    }
};

class EnemyTank {
public:
    int x, y, dirX, dirY, moveDelay, shootDelay;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    bool active;
    vector<Bullet> bullets;

    EnemyTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* bTex) {
        bulletTexture = bTex;
        moveDelay = 15;
        shootDelay = 5;
        x = startX;
        y = startY;
        rect = { x, y, TILE_SIZE, TILE_SIZE };
        dirX = 0;
        dirY = 1;
        active = true;
        SDL_Surface* surface = IMG_Load("anh222.png");
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void move(const vector<Wall>& walls) {
        if (--moveDelay > 0) return;
        moveDelay = 15;
        int r = rand() % 4;
        dirX = (r == 2) ? -1 : (r == 3) ? 1 : 0;
        dirY = (r == 0) ? -1 : (r == 1) ? 1 : 0;
        int newX = x + dirX * 5;
        int newY = y + dirY * 5;

        SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE };
        for (const auto& wall : walls)
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) return;

        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void shoot() {
        if (--shootDelay > 0) return;
        shootDelay = 5;
        bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture));
    }

    void updateBullets() {
        for (auto& b : bullets) b.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
        double angle = 0.0;
        if (dirX == 0 && dirY == -1) angle = 0.0;
        else if (dirX == 1 && dirY == 0) angle = 90.0;
        else if (dirX == 0 && dirY == 1) angle = 180.0;
        else if (dirX == -1 && dirY == 0) angle = 270.0;
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);

        for (auto& bullet : bullets) bullet.render(renderer);
    }
};

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* bulletTexture;
    SDL_Texture* wallTexture;
    SDL_Texture* floorTexture;
    bool running;
    vector<Wall> walls;
    PlayerTank player;
    int enemyNumber = 3;
    vector<EnemyTank> enemies;

    // Thêm các bi?n âm thanh
    Mix_Chunk* shootSound;
    Mix_Chunk* explosionSound;
    Mix_Music* gameMusic;

    Game() {
        running = true;
        if (SDL_Init(SDL_INIT_VIDEO) < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) || Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
            cerr << "SDL Init or IMG_Init or Mix_Init failed\n";
            running = false;
            return;
        }

        if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
            cerr << "Mix_OpenAudio failed: " << Mix_GetError() << endl;
            running = false;
            return;
        }

        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        bulletTexture = IMG_LoadTexture(renderer, "bullet.png");
        wallTexture = IMG_LoadTexture(renderer, "brick.png");
        floorTexture = IMG_LoadTexture(renderer, "floor1.png");

        // T?i âm thanh
        shootSound = Mix_LoadWAV("shoot1.wav");
        explosionSound = Mix_LoadWAV("explosion.wav");
        gameMusic = Mix_LoadMUS("gamemusic.mp3");

        if (!bulletTexture || !wallTexture || !floorTexture || !shootSound || !explosionSound || !gameMusic) {
            cerr << "Failed to load textures or sounds\n";
            running = false;
            return;
        }
        Mix_VolumeMusic(32);
        // B?t d?u phát nh?c n?n
        Mix_PlayMusic(gameMusic, -1); // -1 d? l?p l?i mãi mãi

        generateWalls();
        player = PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, bulletTexture);
        spawnEnemies();
    }

    void generateWalls() {
        for (int i = 3; i < MAP_HEIGHT - 3; i += 2) {
            for (int j = 3; j < MAP_WIDTH - 3; j += 2) {
                walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE);
            }
        }
    }

    void spawnEnemies() {
        enemies.clear();
        for (int i = 0; i < enemyNumber; ++i) {
            int ex, ey;
            bool valid = false;
            while (!valid) {
                ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
                ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
                valid = true;
                for (const auto& wall : walls)
                    if (wall.active && wall.x == ex && wall.y == ey)
                        valid = false;
            }
            enemies.emplace_back(ex, ey, renderer, bulletTexture);
        }
    }

    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_UP: player.move(0, -5, walls); break;
                case SDLK_DOWN: player.move(0, 5, walls); break;
                case SDLK_LEFT: player.move(-5, 0, walls); break;
                case SDLK_RIGHT: player.move(5, 0, walls); break;
                case SDLK_SPACE:
                    player.shoot();
                    Mix_PlayChannel(-1, shootSound, 0); // Phát âm thanh b?n
                    break;
                }
            }
        }
    }

    void update() {
        player.updateBullets();
        for (auto& e : enemies) {
            e.move(walls);
            e.updateBullets();
            if (rand() % 100 < 2) e.shoot();
        }

        for (auto& e : enemies)
            for (auto& b : e.bullets)
                for (auto& wall : walls)
                    if (wall.active && SDL_HasIntersection(&b.rect, &wall.rect)) {
                        wall.active = b.active = false;
                        Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh n?
                    }

        for (auto& b : player.bullets)
            for (auto& wall : walls)
                if (wall.active && SDL_HasIntersection(&b.rect, &wall.rect)) {
                    wall.active = b.active = false;
                    Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh n?
                }

        for (auto& b : player.bullets)
            for (auto& e : enemies)
                if (e.active && SDL_HasIntersection(&b.rect, &e.rect)) {
                    e.active = b.active = false;
                    Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh n?
                }

        enemies.erase(remove_if(enemies.begin(), enemies.end(),
            [](EnemyTank& e) { return !e.active; }), enemies.end());

        for (auto& e : enemies)
            for (auto& b : e.bullets)
                if (SDL_HasIntersection(&b.rect, &player.rect)) {
                    running = false; // Player b? b?n
                }

        if (enemies.empty()) running = false; // K?t thúc n?u không còn k? d?ch
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);

        // V? n?n
        for (int i = 0; i < MAP_HEIGHT; ++i)
            for (int j = 0; j < MAP_WIDTH; ++j) {
                SDL_Rect tile = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, floorTexture, NULL, &tile);
            }

        // V? tu?ng
        for (auto& w : walls) w.render(renderer, wallTexture);

        // V? xe tang, d?n
        player.render(renderer);
        for (auto& e : enemies) e.render(renderer);

        SDL_RenderPresent(renderer);
    }

    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);
        }
    }

    ~Game() {
        Mix_FreeChunk(shootSound);
        Mix_FreeChunk(explosionSound);
        Mix_FreeMusic(gameMusic);
        SDL_DestroyTexture(bulletTexture);
        SDL_DestroyTexture(wallTexture);
        SDL_DestroyTexture(floorTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Game game;
    if (game.running) game.run();
    return 0;
}
