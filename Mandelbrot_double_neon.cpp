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

int Mandelbrot (float* C_Re_Arr1, float* C_Im_Arr1, int* Iterations1,
                float* C_Re_Arr2, float* C_Im_Arr2, int* Iterations2);

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
                float C_Re_Arr1[4], C_Im_Arr1[4];
                float C_Re_Arr2[4], C_Im_Arr2[4];
                int Iterations_1[4], Iterations_2[4];

                for (int i = 0; i < 4; i++)
                {
                    C_Re_Arr1[i] = Left_Bottom_X + (x + i) * Scale;
                    C_Im_Arr1[i] = Left_Bottom_Y + y * Scale;
                }

                for (int i = 0; i < 4; i++)
                {
                    C_Re_Arr2[i] = Left_Bottom_X + (x + 4 + i) * Scale;
                    C_Im_Arr2[i] = Left_Bottom_Y + y * Scale;
                }

                Mandelbrot (C_Re_Arr1, C_Im_Arr1, Iterations_1,
                            C_Re_Arr2, C_Im_Arr2, Iterations_2);

                for (int i = 0; i < 4; i++)
                {
                    int Iteration = Iterations_1[i];
                    int Brightness = 255 - (Iteration * 255 / MAX_ITERATION);
                    Pixels[y * SCREEN_HORIZONTAL + (x + i)] = (Color){(byte_k)Brightness, (byte_k)Brightness, (byte_k)Brightness, 255};

                }

                for (int i = 0; i < 4; i++)
                {
                    int Iteration = Iterations_2[i];
                    float Temp = powf (255 - Iteration * 255 / MAX_ITERATION, 0.9f);
                    // int Brightness =  - Temp;
                    Pixels[y * SCREEN_HORIZONTAL + (x + i)] = (Color){(byte_k)Temp, (byte_k)Temp, (byte_k)Temp, 255};
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


int Mandelbrot (float* C_Re_Arr1, float* C_Im_Arr1, int* Iterations1,
                float* C_Re_Arr2, float* C_Im_Arr2, int* Iterations2)
{
    float32x4_t C_Re1 = vld1q_f32 (C_Re_Arr1);
    float32x4_t C_Im1 = vld1q_f32 (C_Im_Arr1);
    float32x4_t C_Re2 = vld1q_f32 (C_Re_Arr2);
    float32x4_t C_Im2 = vld1q_f32 (C_Im_Arr2);

    float32x4_t Z_n_Re1 = vdupq_n_f32(0.0f);
    float32x4_t Z_n_Im1 = vdupq_n_f32(0.0f);
    float32x4_t Z_n_Re2 = vdupq_n_f32(0.0f);
    float32x4_t Z_n_Im2 = vdupq_n_f32(0.0f);

    int32x4_t Iter1 = vdupq_n_s32 (MAX_ITERATION);
    int32x4_t Iter2 = vdupq_n_s32 (MAX_ITERATION);

    for (int Iter = 0; Iter <= MAX_ITERATION; Iter++)
    {
        float32x4_t Re_sq1 = vmulq_f32 (Z_n_Re1, Z_n_Re1);
        float32x4_t Im_sq1 = vmulq_f32 (Z_n_Im1, Z_n_Im1);
        float32x4_t Re_sq2 = vmulq_f32 (Z_n_Re2, Z_n_Re2);
        float32x4_t Im_sq2 = vmulq_f32 (Z_n_Im2, Z_n_Im2);

        float32x4_t Z_mod_pow_2_1 = vaddq_f32 (Re_sq1, Im_sq1);
        float32x4_t Z_mod_pow_2_2 = vaddq_f32 (Re_sq2, Im_sq2);

        uint32x4_t Mask_1 = vcgtq_f32(Z_mod_pow_2_1, vdupq_n_f32 (4.0f));
        uint32x4_t Mask_2 = vcgtq_f32(Z_mod_pow_2_2, vdupq_n_f32 (4.0f));

        int32x4_t Current_Iter = vdupq_n_s32 (Iter);

        int32x4_t Max_Iter_Vec = vdupq_n_s32(MAX_ITERATION);
        uint32x4_t Need_Update_1 = vandq_u32 (Mask_1, vceqq_s32 (Iter1, Max_Iter_Vec));
        uint32x4_t Need_Update_2 = vandq_u32 (Mask_2, vceqq_s32 (Iter2, Max_Iter_Vec));

        Iter1 = vbslq_s32 (Need_Update_1, Current_Iter, Iter1);
        Iter2 = vbslq_s32 (Need_Update_2, Current_Iter, Iter2);

        uint32x4_t Still_Active_1 = vceqq_s32 (Iter1, Max_Iter_Vec);             // остались ли активные точки
        uint32x4_t Still_Active_2 = vceqq_s32 (Iter2, Max_Iter_Vec);

        if (vaddvq_u32(Still_Active_1) == 0 && vaddvq_u32(Still_Active_2) == 0)
            break;   // 8 точек вылетели

        float32x4_t Z_n_1_Re1 = vaddq_f32 (vsubq_f32 (Re_sq1, Im_sq1), C_Re1);
        float32x4_t Z_n_1_Re2 = vaddq_f32 (vsubq_f32 (Re_sq2, Im_sq2), C_Re2);

        float32x4_t xy1 = vmulq_f32 (Z_n_Re1, Z_n_Im1);
        float32x4_t xy2 = vmulq_f32 (Z_n_Re2, Z_n_Im2);
        float32x4_t Z_n_1_Im1 = vaddq_f32 (vmulq_n_f32 (xy1, 2.0f), C_Im1);
        float32x4_t Z_n_1_Im2 = vaddq_f32 (vmulq_n_f32 (xy2, 2.0f), C_Im2);

        Z_n_Re1 = Z_n_1_Re1;
        Z_n_Im1 = Z_n_1_Im1;
        Z_n_Re2 = Z_n_1_Re2;
        Z_n_Im2 = Z_n_1_Im2;
    }

    vst1q_s32 (Iterations1, Iter1);
    vst1q_s32 (Iterations2, Iter2);

    return 0;
}
