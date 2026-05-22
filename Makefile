CC = clang++

LDFLAGS = -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib \
          -framework OpenGL -framework Cocoa -framework IOKit

SOURCE = mandelbrot_5_fps.cpp
TARGET = out


all: help

no-opt:
	$(CC) -o $(TARGET) mandelbrot_no_opt.cpp $(LDFLAGS)
	./$(TARGET)


buffering-opt: mandelbrot_buffering.cpp
	$(CC) -O3 -o $(TARGET) mandelbrot_buffering.cpp $(LDFLAGS)
	./$(TARGET)


neon-opt: mandelbrot_neon.cpp
	$(CC) -O3 -arch arm64 -ffast-math -mcpu=apple-m4 -mtune=apple-m4 -funroll-loops -flto -o $(TARGET) mandelbrot_neon.cpp -lraylib -lm $(LDFLAGS)
	./$(TARGET)


dneon-opt: mandelbrot_dneon.cpp
	$(CC) -O3 -arch arm64 -ffast-math -mcpu=apple-m4 -mtune=apple-m4 -funroll-loops -flto -o $(TARGET) mandelbrot_dneon.cpp -lraylib -lm $(LDFLAGS)
	./$(TARGET)


qneon-opt: mandelbrot_qneon.cpp
	$(CC) -O3 -arch arm64 -ffast-math -mcpu=apple-m4 -mtune=apple-m4 -funroll-loops -flto -o $(TARGET) mandelbrot_qneon.cpp -lraylib -lm $(LDFLAGS)
	./$(TARGET)


sme-opt: mandelbrot_sme.cpp
	$(CC) -O3 -march=armv9-a+sme -DARM_SME -o $(TARGET) mandelbrot_sme.cpp $(LDFLAGS)
	./$(TARGET)


clean:
	rm -f $(TARGET)
	clear
	clear
