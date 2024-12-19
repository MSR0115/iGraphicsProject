#include "iGraphics.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define screenwidth 600
#define screenheight 600
#define rows 10
#define cols 17
#define brickWidth 30
#define brickHeight 10
#define brickPadding 1
#define maxpowerup 5
#define maxball 4
#define maxscores 10

int bricks[rows][cols] = {0};

bool home = false, instruction = false, credit = false, level = false, start = false, game_over = false, paused = true, entered_name = true, score_board = false;
bool move_left = false, move_right = false;

int paddle_x = 200, paddle_y = 50, paddle_width = 100, paddle_height = 10, paddle_speed = 5;
int ball_x = 250, ball_y = paddle_y + paddle_height + 5, ball_radius = 5, ball_dx = 3, ball_dy = 3, ball_speed = 5;
float hit_point = (ball_x - paddle_x) / paddle_width;

int brick_x = 0, brick_y = 0;

bool music = true, music_brick_ball = false;

int easy = 1, medium = 0, hard = 0, lives = 3, score = 0;
char scor[1000];
char liv[100];

typedef struct
{
    int x, y;
    int dx, dy;
    bool active;
} ball;

ball balls[maxball];

typedef struct
{
    int x, y;
    int type;
    bool active;
} PowerUp;

PowerUp powerup[maxpowerup];

typedef struct
{
    char name[50];
    int score;
} Player;

char playername[50];
int nameindex = 0;

Player scores[maxscores];

void checkBrickCollision();

void load_score()
{
    FILE *file = fopen("highscores.txt", "r");
    if (file == NULL)
    {
        for (int i = 0; i < maxscores; i++)
        {
            strcpy(scores[i].name, "PLAYER");
            scores[i].score = 0;
        }
        return;
    }
    for (int i = 0; i < maxscores; i++)
    {
        if (fscanf(file, "%s %d", scores[i].name, &scores[i].score) != 2)
        {
            break;
        }
    }
    fclose(file);
    return;
}

void save_score()
{
    FILE *file = fopen("highscores.txt", "w");
    assert(file != NULL);

    for (int i = 0; i < maxscores; i++)
    {
        fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
    }
    fclose(file);
    return;
}

void update_score(char *name, int newscore)
{
    int i = 0;
    int j = maxscores - 1;
    load_score();
    for (; i < maxscores; i++)
    {
        if (newscore > scores[i].score)
        {
            for (; j > i; j--)
            {
                scores[j] = scores[j - 1];
            }
            strncpy(scores[i].name, name, sizeof(scores[i].name) - 1);
            scores[i].name[sizeof(scores[i].name) - 1] = '\0';
            scores[i].score = newscore;
            break;
            break;
        }
    }
    save_score();
    return;
}

void draw_scores()
{
    iShowBMP(0, 0, "dxball\\leaderboard.bmp");

    for (int i = 0; i < maxscores; i++)
    {
        char score_entry[100];
        sprintf(score_entry, "%d.%s - %d", i + 1, scores[i].name, scores[i].score);
        iText(200, 450 - i * 30, score_entry, GLUT_BITMAP_TIMES_ROMAN_24);
    }
    iText(10, 550, "<<BACK", GLUT_BITMAP_TIMES_ROMAN_24);
}

void adjustlevel()
{
    if (easy)
    {
        paddle_width = 100;
        ball_speed = 4;
        paddle_speed = 5;
    }
    else if (medium)
    {
        paddle_width = 95;
        ball_speed = 5;
        paddle_speed = 6;
    }
    else
    {
        paddle_width = 90;
        ball_speed = 5;
        paddle_speed = 7;
    }

    return;
}

void initbrick0()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            bricks[i][j] = 0;
        }
    }
}

void initBricks()
{
    initbrick0();
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {

            if (easy)
            {
                if ((i + j) % 2 == 0)
                {
                    bricks[i][j] = 1;
                }
            }
            else if (medium)
            {
                if ((i + j) % 2 == 0)
                {
                    bricks[i][j] = 1;
                }
                else if ((i == rows - 1))
                {
                    bricks[i][j] = 2;
                }
            }
            else
            {
                if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1)
                {
                    bricks[i][j] = 3;
                }
                else if ((i + j) % 2 == 0)
                {
                    bricks[i][j] = 2;
                }
                else
                {
                    bricks[i][j] = 1;
                }
            }
        }
    }
}

void initPowerup()
{
    for (int i = 0; i < maxpowerup; i++)
    {
        powerup[i].active = false;
    }
}

void init_balls()
{
    for (int i = 0; i < maxball; i++)
    {
        balls[0].x = paddle_x + paddle_width / 2;
        balls[0].y = paddle_y + paddle_height + 5;
        balls[0].dx = ball_dx;
        balls[0].dy = ball_dy;
        balls[0].active = true;
    }
}

bool win()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (bricks[i][j] > 0)
            {
                return false;
            }
        }
    }
    return true;
}

void update_paddle_position()
{
    if (move_left && paddle_x > 0)
    {
        paddle_x -= paddle_speed;
        if (paused)
        {
            balls[0].x -= paddle_speed;
        }
    }
    if (move_right && paddle_x + paddle_width < screenwidth)
    {
        paddle_x += paddle_speed;
        if (paused)
        {
            balls[0].x += paddle_speed;
        }
    }
}

void drawPaddle()
{
    iSetColor(229, 186, 14);
    iFilledRectangle(paddle_x, paddle_y, paddle_width, paddle_height);
}

void drawBall()
{
    iSetColor(229, 14, 31);
    iFilledCircle(ball_x, ball_y, ball_radius);
}

void drawPowerUps()
{
    for (int i = 0; i < maxpowerup; i++)
    {
        if (powerup[i].active)
        {
            if (powerup[i].type == 1)
            {
                if (hard)
                {
                    iShowBMP2(powerup[i].x, powerup[i].y, "dxball\\question.bmp", 0);
                }
                else
                {
                    iShowBMP(powerup[i].x, powerup[i].y, "dxball\\lives.bmp");
                }
            }
            else if (powerup[i].type == 3)
            {
                if (hard)
                {
                    iShowBMP2(powerup[i].x, powerup[i].y, "dxball\\question.bmp", 0);
                }
                else
                {
                    iShowBMP(powerup[i].x, powerup[i].y, "dxball\\paddle.bmp");
                }
            }
            else if (powerup[i].type == 2)
            {
                if (hard)
                {
                    iShowBMP2(powerup[i].x, powerup[i].y, "dxball\\question.bmp", 0);
                }
                else
                {

                    iShowBMP(powerup[i].x, powerup[i].y, "dxball\\speed.bmp");
                }
            }
            else if (powerup[i].type == 4)
            {
                if (hard)
                {
                    iShowBMP2(powerup[i].x, powerup[i].y, "dxball\\question.bmp", 0);
                }
                else
                {

                    iShowBMP(powerup[i].x, powerup[i].y, "dxball\\mltball.bmp");
                }
            }
            else if (powerup[i].type == 5 && hard)
            {
                iShowBMP2(powerup[i].x, powerup[i].y, "dxball\\question.bmp", 0);
            }
        }
    }
}

void spawnPowerup(int x, int y)
{
    for (int i = 0; i < maxpowerup; i++)
    {
        if (!powerup[i].active)
        {
            powerup[i].x = x;
            powerup[i].y = y;
            powerup[i].type = rand() % 5 + 1;
            powerup[i].active = true;
            break;
        }
    }
}

void active_ball()
{

    int count = 1;
    for (int i = 0; i < maxball; i++)
    {
        if (!balls[i].active && count < maxball)
        {
            balls[i].x = balls[0].x;
            balls[i].y = balls[0].y;
            balls[i].dx = (i % 2 == 0) ? -ball_dx : ball_dx;
            balls[i].dy = ball_dy;
            balls[i].active = true;
            count++;
        }
    }
}

void updatePowerup()
{
    for (int i = 0; i < maxpowerup; i++)
    {
        if (powerup[i].active)
        {
            powerup[i].y -= 2;

            if (powerup[i].y <= paddle_y + paddle_height && powerup[i].y >= paddle_y && powerup[i].x >= paddle_x && powerup[i].x <= paddle_x + paddle_width)
            {
                if (powerup[i].type == 1)
                {
                    lives++;
                }
                else if (powerup[i].type == 2)
                {
                    ball_dx += 1;
                    ball_dy += 1;
                }
                else if (powerup[i].type == 3)
                {
                    paddle_width += 5;
                }
                else if (powerup[i].type == 4)
                {
                    active_ball();
                }
                else if (powerup[i].type == 5 && hard)
                {
                    if (paddle_width > 70)
                    {
                        paddle_width -= 5;
                    }
                }

                powerup[i].active = false;
            }

            if (powerup[i].y < 0)
            {
                powerup[i].active = false;
            }
        }
    }
}

void ball_move()
{
    bool b_active = false;
    for (int i = 0; i < maxball; i++)
    {
        if (balls[i].active)
        {
            balls[i].x += balls[i].dx;
            balls[i].y += balls[i].dy;

            if (balls[i].x <= 0 || balls[i].x >= screenwidth)
            {
                balls[i].dx = -balls[i].dx;
            }
            if (balls[i].y >= screenheight)
            {
                balls[i].dy = -balls[i].dy;
            }
            if (balls[i].y >= paddle_y && balls[i].y <= paddle_y + paddle_height &&
                balls[i].x >= paddle_x && balls[i].x <= paddle_x + paddle_width)
            {

                int con = (paddle_x + paddle_width - balls[i].x) + 40;
                balls[i].dx = (int)ball_speed * cos(con * 3.1416 / 180);
                balls[i].dy = (int)ball_speed * sin(con * 3.1416 / 180);
            }
        }

        if (balls[i].y < 0)
        {
            balls[i].active = false;
        }

        for (int i = 0; i < maxball; i++)
        {
            if (balls[i].active)
            {
                b_active = true;
            }
        }
        if (!b_active)
        {

            lives--;
            if (lives <= 0)
            {
                start = false;
                game_over = true;
                if (game_over)
                {
                    PlaySound("music\\gameOver.wav", NULL, SND_ASYNC);
                }
                update_score(playername, score);
                break;
            }
            else
            {
                init_balls();
                paused = true;
            }
        }
    }

    updatePowerup();
    checkBrickCollision();
}

void instruction_page()
{
    iShowBMP2(0, 0, "dxball\\inst.bmp", 0);
    iSetColor(255, 0, 0);
    iText(10, 550, "<<BACK", GLUT_BITMAP_TIMES_ROMAN_24);
}

void home_page()
{
    iShowBMP2(0, 0, "dxball\\home.bmp", 0);
}

void level_page()
{
    iShowBMP2(0, 0, "dxball\\level.bmp", 0);
    iSetColor(255, 0, 0);
    iText(10, 550, "<<BACK", GLUT_BITMAP_TIMES_ROMAN_24);
}

void credit_page()
{
    iShowBMP2(0, 0, "dxball\\dx1 (1).bmp", 0);
    iSetColor(255, 0, 0);
    iText(10, 550, "<<BACK", GLUT_BITMAP_TIMES_ROMAN_24);
}

void gameover_page()
{
    iShowBMP2(0, 0, "dxball\\gameover.bmp", 0);
    iSetColor(255, 0, 0);
    iText(200, 300, "SCORE:", GLUT_BITMAP_TIMES_ROMAN_24);
    sprintf(scor, "%d", score);
    iText(290, 300, scor, GLUT_BITMAP_TIMES_ROMAN_24);
    if (win())
    {
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, 600, 600);
        iShowBMP2(0, 0, "dxball\\win.bmp", 0);
    }
    iSetColor(255, 0, 0);
    iText(10, 550, "<<BACK", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawBricks()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (bricks[i][j] > 0)
            {
                brick_x = (j + 1) * (brickWidth + brickPadding);
                brick_y = screenheight - (i + 1) * (brickHeight + brickPadding) - 100;

                if (bricks[i][j] == 3)
                {
                    iSetColor(255, 0, 0);
                }
                else if (bricks[i][j] == 2)
                {
                    iSetColor(255, 255, 0);
                }
                else
                {
                    iSetColor(81, 238, 60);
                }

                iFilledRectangle(brick_x, brick_y, brickWidth, brickHeight);
            }
        }
    }
}

void checkBrickCollision()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (bricks[i][j] > 0)
            {
                brick_x = (j + 1) * (brickWidth + brickPadding);
                brick_y = screenheight - 100 - (i + 1) * (brickHeight + brickPadding);

                for (int k = 0; k < maxball; k++)
                {
                    if (balls[k].active)
                    {
                        if (balls[k].x + ball_radius > brick_x && balls[k].x - ball_radius < brick_x + brickWidth &&
                            balls[k].y + ball_radius > brick_y && balls[k].y - ball_radius < brick_y + brickHeight)
                        {
                            if (balls[k].y < brick_y + brickHeight && balls[k].y > brick_y)
                            {
                                balls[k].dx = -balls[k].dx;
                            }
                            else
                            {
                                balls[k].dy = -balls[k].dy;
                            }

                            PlaySound("music\\DX Ball Game_sounds_BallAndBrick.wav", NULL, SND_ASYNC);

                            bricks[i][j]--;

                            if (bricks[i][j] == 0)
                            {
                                score++;
                                music_brick_ball = true;

                                //
                                int prob = 0;
                                if (easy)
                                {
                                    prob = rand() % 5;
                                }
                                else if (medium)
                                {
                                    prob = rand() % 7;
                                }
                                else
                                {
                                    prob = rand() % 9;
                                }

                                if (prob == 0)
                                {
                                    spawnPowerup(brick_x + brickWidth / 2, brick_y);
                                }
                            }

                            if (win())
                            {
                                update_score(playername, score);
                                start = false;
                                game_over = true;
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

void iDraw()
{

    iClear();

    if (entered_name)
    {
        iShowBMP2(0, 0, "dxball\\name.bmp", 0);
        iSetColor(255, 0, 0);
        iText(130, 320, playername, GLUT_BITMAP_TIMES_ROMAN_24);
    }

    if (score_board)
    {
        draw_scores();
    }

    if (home)
    {
        home_page();
    }
    if (instruction)
    {
        instruction_page();
    }
    if (credit)
    {
        credit_page();
    }
    if (start)
    {
        music = false;
        iShowBMP(0, 0, "dxball\\bg.bmp");
        iText(10, 570, "Score:", GLUT_BITMAP_HELVETICA_18);
        sprintf(scor, "%d", score);
        iText(70, 570, scor, GLUT_BITMAP_HELVETICA_18);

        iText(120, 570, "lives:", GLUT_BITMAP_HELVETICA_18);
        for (int i = 0; i < lives; i++)
        {
            iShowBMP2(165 + i * 30, 565, "dxball\\lives.bmp", 0);
        }

        drawBricks();

        drawPaddle();

        for (int i = 0; i < maxball; i++)
        {
            if (balls[i].active)
            {
                iSetColor(255, 0, 0);
                iFilledCircle(balls[i].x, balls[i].y, 5, 100);
            }
        }

        drawPowerUps();

        if (paused)
        {
            iShowBMP2(200, 200, "dxball\\pause.bmp", 0);
        }
    }
    if (level)
    {
        level_page();
    }
    if (game_over)
    {
        gameover_page();
    }
}

void iMouseMove(int mx, int my)
{
    // printf("x = %d, y= %d\n", mx, my);

    // place your codes here
}

void iMouse(int button, int state, int mx, int my)
{
    if (home)
    {

        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 108 && my < 150)
            {
                home = !home;
                instruction = !instruction;
            }
            if (my > 152 && my < 193)
            {
                home = !home;
                credit = !credit;
            }
            if (my > 50 && my < 95)
            {
                exit(0);
            }
            if (my > 300 && my < 362)
            {
                start = !start;
                home = !home;
                initBricks();
                adjustlevel();
            }
            if (my > 250 && my < 286)
            {
                level = !level;
                home = !home;
            }
            if (my > 206 && my < 243)
            {
                load_score();
                score_board = !score_board;
                home = !home;
            }
        }
    }
    if (instruction)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                home = !home;
                instruction = !instruction;
            }
        }
    }
    if (credit)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                home = !home;
                credit = !credit;
            }
        }
    }
    if (level)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 383 && my < 444)
            {
                easy = 1;
                medium = 0;
                hard = 0;
                score = 0;
                lives = 3;
                paused = true;
                initBricks();
                init_balls();
                init_balls();
                initPowerup();
            }
            if (my > 292 && my < 362)
            {
                easy = 0;
                medium = 1;
                hard = 0;
                score = 0;
                lives = 3;
                paused = true;
                initBricks();
                init_balls();
                init_balls();
                initPowerup();
            }
            if (my > 188 && my < 270)
            {
                easy = 0;
                medium = 0;
                hard = 1;
                score = 0;
                lives = 3;
                paused = true;
                initBricks();
                init_balls();
                init_balls();
                initPowerup();
            }
        }
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                home = !home;
                level = !level;
            }
        }
    }
    if (score_board)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                score_board = !score_board;
                home = !home;
            }
        }
    }

    if (score_board)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                score_board = !score_board;
                home = !home;
            }
        }
    }
    if (game_over)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if (my > 550)
            {
                game_over = !game_over;
                home = !home;
                initBricks();
                init_balls();
                init_balls();
                initPowerup();
                music = true;
                lives = 3;
                score = 0;
                paused = true;
                PlaySound("music\\gameOpening.wav", NULL, SND_LOOP | SND_ASYNC);
            }
        }
    }
}

void iKeyboard(unsigned char key)
{
    if (entered_name)
    {
        if (key == '\r')
        {
            entered_name = !entered_name;
            home = !home;
        }
        else if (key == '\b')
        {
            if (nameindex > 0)
            {
                nameindex--;
                playername[nameindex] = '\0';
            }
        }
        else if (nameindex < 49)
        {
            playername[nameindex] = key;
            nameindex++;
            playername[nameindex] = '\0';
        }
    }

    if (start)
    {

        if (key == 'a')
        {
            move_left = true;
            move_right = false;
        }
        if (key == 'd')
        {
            move_right = true;
            move_left = false;
        }
        if (key == 'w')
        {
            move_left = false;
            move_right = false;
        }
        if (key == 'e')
        {
            start = !start;
            home = !home;
        }
        if (key == 'p' || key == ' ')
        {
            paused = !paused;
        }
        if (paused)
        {
            if (key == 'a' && paddle_x > 0)
            {
                paddle_x = paddle_x - paddle_speed;
                if (paused)
                {
                    balls[0].x = balls[0].x - paddle_speed;
                }
            }
            if (key == 'd' && paddle_x + paddle_width < screenwidth)
            {
                paddle_x = paddle_x + paddle_speed;
                if (paused)
                {
                    balls[0].x = balls[0].x + paddle_speed;
                }
            }
        }
    }

    if (game_over)
    {
        if (key == 'e')
        {
            game_over = !game_over;
            home = !home;
            initBricks();
            init_balls();
            init_balls();
            initPowerup();
            music = true;
            lives = 3;
            score = 0;
            paused = true;
            PlaySound("music\\gameOpening.wav", NULL, SND_LOOP | SND_ASYNC);
        }
    }

    if (key == 'q')
    {
        exit(0);
    }
}

void iSpecialKeyboard(unsigned char key)
{

    if (key == GLUT_KEY_END)
    {
        exit(0);
    }

    if (key == GLUT_KEY_F10)
    {
        if (music)
        {
            music = false;
            PlaySound(0, 0, 0);
        }
        else
        {
            music = true;
            PlaySound("music\\gameOpening.wav", NULL, SND_LOOP | SND_ASYNC);
        }
    }
    if (start)
    {
        if (key == GLUT_KEY_LEFT)
        {
            move_left = true;
            move_right = false;
        }
        if (key == GLUT_KEY_RIGHT)
        {
            move_right = true;
            move_left = false;
        }
        if (key == GLUT_KEY_UP)
        {
            move_left = false;
            move_right = false;
        }

        if (paused)
        {
            if (key == GLUT_KEY_LEFT && paddle_x > 0)
            {
                paddle_x = paddle_x - paddle_speed;
                if (paused)
                {
                    balls[0].x = balls[0].x - paddle_speed;
                }
            }
            if (key == GLUT_KEY_RIGHT && paddle_x + paddle_width < screenwidth)
            {
                paddle_x = paddle_x + paddle_speed;
                if (paused)
                {
                    balls[0].x = balls[0].x + paddle_speed;
                }
            }
        }
    }
}

void change()
{

    if (start && !paused)
    {
        ball_move();
        update_paddle_position();
        win();
    }
}

int main()
{
    init_balls();
    initPowerup();
    iSetTimer(5, change);

    if (music)
    {
        PlaySound("music\\gameOpening.wav", NULL, SND_LOOP | SND_ASYNC);
    }

    iInitialize(screenwidth, screenheight, "2305090-DX BALL Basic");
    return 0;
}
