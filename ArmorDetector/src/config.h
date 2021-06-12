#ifndef CONFIG_H
#define CONFIG_H

// The file name of ".bin" and ".param" file
#define MODEL_PATH        "demo"

// The size selected when training model
#define INPUT_SIZE        480

#define USE_GPU           true

// Integer -> Webcam id, OR String -> video path
#define CAPTURE_SOURCE    0

// Bounding box whose score under SCORE_THRESHOLD will be ignored
#define SCORE_THRESHOLD   0.5

// Avoid multi-boxes on one object
#define NMS_THRESHOLD     0.7

#define CLASS_NAMES "BLU1", "RED1", "BLU2", "RED2", "BLU3", "RED3", \
                    "BLU4", "RED4", "BLU5", "RED5", "BLU7", "RED7", \
                    "BLU8", "RED8"

#define BLU 0
#define RED 1

#define ENEMY_COLOR BLU

#endif // CONFIG_H
