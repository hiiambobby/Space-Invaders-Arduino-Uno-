#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define LED_PIN 2
#define NUM_PIXELS 256
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define ENEMY_COUNT 8
#define MAX_BULLETS 5
#define MAX_ENEMY_BULLETS 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
#define BUTTON_SHOOT 3

Adafruit_NeoPixel matrix(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool gameOver = false;
bool matrixChanged = false;
void markMatrixChanged() {
    matrixChanged = true;  
}
void triggerGameOver(uint32_t endColor) {
    gameOver = true;
    for (int i = 0; i < NUM_PIXELS; i++) {
        matrix.setPixelColor(i, endColor);
    }
    matrix.show();
    matrixChanged = true;

    delay(200);  
    resetGame(); 
}
class GameObject
{
protected:
    int x, y;
    uint32_t color;
    bool active;

public:
    GameObject(int startX, int startY, uint32_t startColor)
        : x(startX), y(startY), color(startColor), active(true) {}

    virtual void draw()
    {
        if (active)
        {
            matrix.setPixelColor(y * MATRIX_WIDTH + x, color);
        }
    }

    virtual void clear()
    {
        if (active)
        {
            matrix.setPixelColor(y * MATRIX_WIDTH + x, 0);
        }
    }

    bool isActive() const { return active; }
    void deactivate()
    {
        active = false;
        clear();
    }

    int getX() const { return x; }
    int getY() const { return y; }
};

class Spaceship : public GameObject
{
public:
    Spaceship() : GameObject(7, 15, matrix.Color(0, 255, 0)) {}

    void draw() override
    {
        if (active)
        {
            matrix.setPixelColor(y * MATRIX_WIDTH + x, color);
            if (x > 0)
                matrix.setPixelColor(y * MATRIX_WIDTH + (x - 1), color);
            if (x < MATRIX_WIDTH - 1)
                matrix.setPixelColor(y * MATRIX_WIDTH + (x + 1), color);
            if (y > 0)
            {
                matrix.setPixelColor((y - 1) * MATRIX_WIDTH + x, color);
            }
        }
    }

    void move(int dx, int dy)
    {
        clear();
        x = constrain(x + dx, 0, MATRIX_WIDTH - 1);
        y = constrain(y + dy, 0, MATRIX_HEIGHT - 1);
        draw();
    matrixChanged = true;
        }

    void clear() override
    {
        if (active)
        {
            matrix.setPixelColor(y * MATRIX_WIDTH + x, 0);
            if (x > 0)
                matrix.setPixelColor(y * MATRIX_WIDTH + (x - 1), 0);
            if (x < MATRIX_WIDTH - 1)
                matrix.setPixelColor(y * MATRIX_WIDTH + (x + 1), 0);
            if (y > 0)
            {
                matrix.setPixelColor((y - 1) * MATRIX_WIDTH + x, 0);
            }
        }
    }
};

class Bullet : public GameObject
{
public:
    Bullet(uint32_t bulletColor) : GameObject(-1, -1, bulletColor) {}

    void shoot(int startX, int startY)
    {
        if (!active)
        {
            x = startX;
            y = startY;
            active = true;
        }
    }

    void update(bool moveUp)
    {
        if (active)
        {
            clear();
            if (moveUp)
            {
                if (y > 0)
                {
                    y--;
                }
                else
                {
                    deactivate();
                }
            }
            else
            {
                if (y < MATRIX_HEIGHT - 1)
                {
                    y++;
                }
                else
                {
                    deactivate();
                }
            }
            draw();
        }
    }
};

class Enemy : public GameObject
{
private:
    bool directionRight;

public:
    Enemy() : GameObject(-1, -1, matrix.Color(255, 144, 0))
    {
        directionRight = true; 
    }

    int getX() { return x; }
    int getY() { return y; }
    void setPosition(int startX, int startY, uint32_t startColor, bool startDirection)
    {
        x = startX;
        y = startY;
        color = startColor;
        active = true;
        directionRight = startDirection; 
    }

    void move()
    {
        if (active)
        {
            clear();
            if (directionRight)
            {
                x++;
                if (x >= MATRIX_WIDTH)
                {
                    x = MATRIX_WIDTH - 1;
                    directionRight = false;
                }
            }
            else
            {
                x--;
                if (x < 0)
                {
                    x = 0;
                    directionRight = true;
                }
            }
            draw();
        }
    }
};

Spaceship spaceship;
Bullet bullets[MAX_BULLETS] = {Bullet(matrix.Color(34, 139, 34)), Bullet(matrix.Color(34, 139, 34)), Bullet(matrix.Color(34, 139, 34)), Bullet(matrix.Color(34, 139, 34)), Bullet(matrix.Color(34, 139, 34))};
Bullet enemyBullets[MAX_ENEMY_BULLETS] = {Bullet(matrix.Color(255, 0, 0)), Bullet(matrix.Color(255, 0, 0)), Bullet(matrix.Color(255, 0, 0))};
Enemy enemies[ENEMY_COUNT];


//Enemy movement method 1

void placeEnemies() {
    int startX = 2; // Minimum starting x-coordinate
    int startY = 0; // Minimum starting y-coordinate
    int maxX = MATRIX_WIDTH - 2; 
    int maxY = 4; 
    bool initialDirection = true; 

    for (int i = 0; i < ENEMY_COUNT; i++) {
        int posX = random(startX, maxX);
        int posY = random(startY, maxY);
        bool validPosition = true;
        for (int j = 0; j < i; j++) {
            if (enemies[j].getX() == posX && enemies[j].getY() == posY) {
                validPosition = false;
                break;
            }
        }
        if (!validPosition) {
            i--; 
            continue;
        }
        enemies[i].setPosition(posX, posY, matrix.Color(255, 140, 0), initialDirection);
        enemies[i].draw();
    }
}

////////////////////////////////////
///Enemy movement method 2/////////
///uncomment this method to use///
/////////////////////////////////

// void placeEnemies()
// {
//     int rows = 2;                  // Number of rows
//     int cols = ENEMY_COUNT / rows; // Number of columns
//     int startX = 2;                // Starting x-coordinate
//     int startY = 0;                // Starting y-coordinate
//     int spacingX = 3;              // Horizontal spacing between enemies
//     int spacingY = 1;              // Vertical spacing between rows
//     bool initialDirection = true;  // All enemies start moving right

//     for (int i = 0; i < ENEMY_COUNT; i++)
//     {
//         int row = i / cols;
//         int col = i % cols;
//         int posX = startX + col * spacingX;
//         int posY = startY + row * spacingY;
//         enemies[i].setPosition(posX, posY, matrix.Color(255, 0, 0), initialDirection);
//         enemies[i].draw();
//     }
// }
void updateBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].update(true);
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        enemyBullets[i].update(false);
    }
}

void checkCollisions()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].isActive())
        {
            int bulletX = bullets[i].getX();
            int bulletY = bullets[i].getY();

            for (int j = 0; j < ENEMY_COUNT; j++)
            {
                if (enemies[j].isActive() && enemies[j].getX() == bulletX && enemies[j].getY() == bulletY)
                {
                    enemies[j].deactivate();
                    bullets[i].deactivate();
                        matrixChanged = true;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        if (enemyBullets[i].isActive())
        {
            int bulletX = enemyBullets[i].getX();
            int bulletY = enemyBullets[i].getY();

            // Check if the bullet hits any pixel of the spaceship
            if ((bulletX == spaceship.getX() && bulletY == spaceship.getY()) ||
                (bulletX == spaceship.getX() - 1 && bulletY == spaceship.getY() && spaceship.getX() > 0) ||
                (bulletX == spaceship.getX() + 1 && bulletY == spaceship.getY() && spaceship.getX() < MATRIX_WIDTH - 1) ||
                (bulletX == spaceship.getX() && bulletY == spaceship.getY() - 1 && spaceship.getY() > 0))
            {
                triggerGameOver(matrix.Color(255, 0, 0));
                break;
            }
        }
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (enemies[i].isActive() && enemies[i].getX() == spaceship.getX() && enemies[i].getY() == spaceship.getY())
        {
            triggerGameOver(matrix.Color(255, 0, 0));
            break;
        }
    }

    if (gameOver)
        return;

    bool enemiesLeft = false;
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (enemies[i].isActive())
        {
            enemiesLeft = true;
            break;
        }
    }

    if (!enemiesLeft)
    {
        triggerGameOver(matrix.Color(0, 255, 0));
    }
}

void shootBullet()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].isActive())
        {
            bullets[i].shoot(spaceship.getX(), spaceship.getY() - 2);

            matrixChanged = true;
            break;
        }
    }
}

void enemyShoot()
{
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        if (!enemyBullets[i].isActive())
        {
            int shooter;
            do
            {
                shooter = random(0, ENEMY_COUNT);
            } while (!enemies[shooter].isActive());
            enemyBullets[i].shoot(enemies[shooter].getX(), enemies[shooter].getY() + 1);
                matrixChanged = true;
            break;
        }
    }
}

void updateEnemies()
{
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        enemies[i].move();
            matrixChanged = true;
    }
}


void updateMatrix()
{
    if (!gameOver)
    {
        updateEnemies();
        spaceship.draw();
        updateBullets();
        checkCollisions();

        if (matrixChanged)
        {
            matrix.show();
            matrixChanged = false;
        }
    }
}


bool isButtonPressed(int pin)
{
    return digitalRead(pin) == HIGH;
}
void clearScreen() {
    // Loop through all pixels and set them to black (off)
    for (int i = 0; i < NUM_PIXELS; i++) {
        matrix.setPixelColor(i, 0);  // Turn off the pixel
    }
    matrix.show();  // Update the matrix
}

void resetGame() {
  clearScreen();
  
    spaceship.deactivate();
    spaceship = Spaceship();  // Recreate spaceship at initial position
    
    // Reset enemy positions
    for (int i = 0; i < ENEMY_COUNT; i++) {
        enemies[i].deactivate();
    }
    placeEnemies();  // Place enemies again
    
    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].deactivate();
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        enemyBullets[i].deactivate();
    }

    // Reset game state
    gameOver = false;
    matrixChanged = true;  
}

void setup()
{
    matrix.begin();
    matrix.show();

    pinMode(BUTTON_UP, INPUT);
    pinMode(BUTTON_DOWN, INPUT);
    pinMode(BUTTON_LEFT, INPUT);
    pinMode(BUTTON_RIGHT, INPUT);
    pinMode(BUTTON_SHOOT, INPUT);

    placeEnemies();
    spaceship.draw();
    updateMatrix();
}


void loop()
{
    if (!gameOver)
    {
        if (isButtonPressed(BUTTON_LEFT))
            spaceship.move(-1, 0);
        if (isButtonPressed(BUTTON_RIGHT))
            spaceship.move(1, 0);
        if (isButtonPressed(BUTTON_UP))
            spaceship.move(0, -1);
        if (isButtonPressed(BUTTON_DOWN))
            spaceship.move(0, 1);
        if (isButtonPressed(BUTTON_SHOOT))
            shootBullet();

        if (random(0, 5) == 0)
            enemyShoot();

        updateMatrix();
        delay(100);
    }
}
