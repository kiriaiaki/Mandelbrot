#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <arm_neon.h>

#define MAX_ITERATION     256

#define SCREEN_HORIZONTAL 800
#define SCREEN_VERTICAL   600

typedef unsigned char byte_k;

int Mandelbrot (float* C_Re, float* C_Im, int* Iterations);

int main ()
{
    float Left_Bottom_X = -2.0f;
    float Left_Bottom_Y = -1.2f;

    float Scale = (1.0f - (-2.0f)) / SCREEN_HORIZONTAL;      // цена деления пикселя

    float Center_X = Left_Bottom_X + (SCREEN_HORIZONTAL / 2) * Scale;
    float Center_Y = Left_Bottom_Y + (SCREEN_VERTICAL   / 2) * Scale;

    InitWindow (SCREEN_HORIZONTAL, SCREEN_VERTICAL, "Mandelbrot");

    Color* Pixels = (Color*) calloc (SCREEN_HORIZONTAL * SCREEN_VERTICAL, sizeof (Color));      // Color - структура из RGB и прозрачности (A)
    if (Pixels == NULL)
    {
        printf ("Erorr allocate memory for pixels!\n");
        CloseWindow ();
        return -1;
    }

    Image Start_Texture = GenImageColor (SCREEN_HORIZONTAL, SCREEN_VERTICAL, BLACK);        // создаем полностью черное изображение
    Texture2D Mandelbrot_Texture = LoadTextureFromImage (Start_Texture);                    // создаем текстуру и загружаем туда изображение (иначе была бы ошибка при обновлении текстуры)
    UnloadImage (Start_Texture);                                                            // удаляем стартовую текстуру


    int* Iterations = (int*) calloc(4, sizeof (int));
    if (Iterations == NULL)
    {
        printf("Error to allocate memory for iters!\n");
        return -1;
    }

    while (!WindowShouldClose())
    {
        float Speed = 0.5f * Scale * 10;

        if (IsKeyDown (KEY_RIGHT)) Left_Bottom_X += Speed;
        if (IsKeyDown (KEY_LEFT) ) Left_Bottom_X -= Speed;
        if (IsKeyDown (KEY_DOWN) ) Left_Bottom_Y += Speed;
        if (IsKeyDown (KEY_UP)   ) Left_Bottom_Y -= Speed;

        if (IsKeyDown(KEY_EQUAL))
        {
            Center_X = Left_Bottom_X + (SCREEN_HORIZONTAL / 2) * Scale;
            Center_Y = Left_Bottom_Y + (SCREEN_VERTICAL   / 2) * Scale;

            Scale *= 0.95f;

            Left_Bottom_X = Center_X - (SCREEN_HORIZONTAL / 2) * Scale;
            Left_Bottom_Y = Center_Y - (SCREEN_VERTICAL   / 2) * Scale;
        }

        if (IsKeyDown(KEY_MINUS))
        {
            Center_X = Left_Bottom_X + (SCREEN_HORIZONTAL / 2) * Scale;
            Center_Y = Left_Bottom_Y + (SCREEN_VERTICAL   / 2) * Scale;

            Scale /= 0.95f;

            Left_Bottom_X = Center_X - (SCREEN_HORIZONTAL / 2) * Scale;
            Left_Bottom_Y = Center_Y - (SCREEN_VERTICAL   / 2) * Scale;
        }

        for (int y = 0; y < SCREEN_VERTICAL; y++)
        {
            for (int x = 0; x < SCREEN_HORIZONTAL; x += 4)
            {
                float C_Re_Arr[4] = {0.0f, 0.0f, 0.0f, 0.0f};
                float C_Im_Arr[4] = {0.0f, 0.0f, 0.0f, 0.0f};

                for (int i = 0; i < 4; i++)
                {
                    C_Re_Arr[i] = Left_Bottom_X + (x + i) * Scale;
                    C_Im_Arr[i] = Left_Bottom_Y + y * Scale;
                }

                Mandelbrot(C_Re_Arr, C_Im_Arr, Iterations);

                for (int i = 0; i < 4; i++)
                {
                    int Iteration = Iterations[i];

                    int Brightness = (Iteration * 255) / MAX_ITERATION;

                    int Gray = (Brightness + ((Brightness * 2) % 255) + ((Brightness * 16) % 255)) / 3;

                    if (Iteration == MAX_ITERATION)
                    {
                        Gray = 0;
                    }
                    else
                    {
                        Gray = 255 - Gray;

                        float Norm = Gray / 255.0f;          // нормализуем в 0..1
                        Norm = powf (Norm, 1.8f);            // гамма > 1 темнит светлые участки
                        Gray = (int) (Norm * 255.0f);
                    }

                    Pixels[(y * SCREEN_HORIZONTAL) + x + i] = (Color) {
                                                                    (byte_k) Gray,
                                                                    (byte_k) Gray,
                                                                    (byte_k) Gray,
                                                                    255
                                                                };
                }
            }
        }

        UpdateTexture (Mandelbrot_Texture, Pixels);     // загружаем массив цветов в видеопамять

        BeginDrawing ();

            ClearBackground (BLACK);
            DrawTexture (Mandelbrot_Texture, 0, 0, WHITE);
            DrawText (TextFormat ("FPS: %d", GetFPS()), 10, 10, 20, GREEN);

            int Origin_X = (int)((-Left_Bottom_X) / Scale);
            int Origin_Y = (int)((-Left_Bottom_Y) / Scale);

            if (Origin_X >= 0 && Origin_X < SCREEN_HORIZONTAL)
            {
                DrawLine(Origin_X, 0, Origin_X, SCREEN_VERTICAL, BLUE);
                DrawText ("Re", SCREEN_HORIZONTAL - 30, Origin_Y - 15, 20, BLUE);
            }
            if (Origin_Y >= 0 && Origin_Y < SCREEN_VERTICAL)
            {
                DrawLine(0, Origin_Y, SCREEN_HORIZONTAL, Origin_Y, BLUE);
                DrawText ("Im",           Origin_X + 5,             5, 20, BLUE);
            }

        EndDrawing   ();
    }

    UnloadTexture (Mandelbrot_Texture);
    free (Pixels);
    CloseWindow ();

    return 0;
}

int Mandelbrot (float* C_Re_Arr, float* C_Im_Arr, int* Iterations)
{
    float32x4_t C_Re = vld1q_f32 (C_Re_Arr);
    float32x4_t C_Im = vld1q_f32 (C_Im_Arr);

    float32x4_t Z_n_Re = vdupq_n_f32 (0.0f);
    float32x4_t Z_n_Im = vdupq_n_f32 (0.0f);

    for (int i = 0; i < 4; i++) Iterations[i] = MAX_ITERATION;

    for (int Iter = 0; Iter <= MAX_ITERATION; Iter++)
    {
        float32x4_t Re_sq = vmulq_f32 (Z_n_Re, Z_n_Re);
        float32x4_t Im_sq = vmulq_f32 (Z_n_Im, Z_n_Im);

        float32x4_t Z_mod_pow_2 = vaddq_f32 (Re_sq, Im_sq);

        uint32x4_t mask = vcgtq_f32(Z_mod_pow_2, vdupq_n_f32(4.0f));

        if (vgetq_lane_u32 (mask, 0) && Iterations[0] == MAX_ITERATION) Iterations[0] = Iter;
        if (vgetq_lane_u32 (mask, 1) && Iterations[1] == MAX_ITERATION) Iterations[1] = Iter;
        if (vgetq_lane_u32 (mask, 2) && Iterations[2] == MAX_ITERATION) Iterations[2] = Iter;
        if (vgetq_lane_u32 (mask, 3) && Iterations[3] == MAX_ITERATION) Iterations[3] = Iter;

        if (Iterations[0] != MAX_ITERATION && Iterations[1] != MAX_ITERATION &&
            Iterations[2] != MAX_ITERATION && Iterations[3] != MAX_ITERATION) break;

        float32x4_t Z_n_1_Re = vaddq_f32 (vsubq_f32 (Re_sq, Im_sq), C_Re);

        float32x4_t xy = vmulq_f32 (Z_n_Re, Z_n_Im);
        float32x4_t Z_n_1_Im = vaddq_f32 (vmulq_n_f32 (xy, 2.0f), C_Im);

        Z_n_Re = Z_n_1_Re;
        Z_n_Im = Z_n_1_Im;
    }

    return MAX_ITERATION;
}
