// Compile with: gcc calibrate_magnetometer.c -o calibrate_magnetometer -lm
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

typedef struct
{
    float x, y, z;
} Vector3D;

float min(float a, float b)
{
    return (a < b) ? a : b;
}

float max(float a, float b)
{
    return (a > b) ? a : b;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <datafile.txt>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "r");
    if(!file)
    {
        perror("Error opening file");
        return 1;
    }

    Vector3D min_vals = {FLT_MAX, FLT_MAX, FLT_MAX};
    Vector3D max_vals = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    float x, y, z, dummy;
    int points_count = 0;
    while(fscanf(file, "%f,%f,%f,%f\n", &x, &y, &z, &dummy) == 4)
    {
        min_vals.x = min(min_vals.x, x);
        min_vals.y = min(min_vals.y, y);
        min_vals.z = min(min_vals.z, z);

        max_vals.x = max(max_vals.x, x);
        max_vals.y = max(max_vals.y, y);
        max_vals.z = max(max_vals.z, z);
        points_count++;
    }
    fclose(file);

    if(points_count == 0)
    {
        fprintf(stderr, "No data points found in the file.\n");
        return 1;
    }

    printf("Processed %d data points.\n\n", points_count);

    Vector3D hard_iron_offset;
    hard_iron_offset.x = (max_vals.x + min_vals.x) / 2.0f;
    hard_iron_offset.y = (max_vals.y + min_vals.y) / 2.0f;
    hard_iron_offset.z = (max_vals.z + min_vals.z) / 2.0f;

    Vector3D soft_iron_scale;
    float avg_radius = ((max_vals.x - min_vals.x) / 2.0f +
                        (max_vals.y - min_vals.y) / 2.0f +
                        (max_vals.z - min_vals.z) / 2.0f) / 3.0f;

    soft_iron_scale.x = avg_radius / ((max_vals.x - min_vals.x) / 2.0f);
    soft_iron_scale.y = avg_radius / ((max_vals.y - min_vals.y) / 2.0f);
    soft_iron_scale.z = avg_radius / ((max_vals.z - min_vals.z) / 2.0f);

    printf("--- Magnetometer Calibration Parameters ---\n");
    printf("--- Copy these values into your microcontroller code and Gnuplot script ---\n\n");

    printf("// Hard-iron offsets\n");
    printf("#define CAL_HI_OFFSET_X  (%.4ff)\n", hard_iron_offset.x);
    printf("#define CAL_HI_OFFSET_Y  (%.4ff)\n", hard_iron_offset.y);
    printf("#define CAL_HI_OFFSET_Z  (%.4ff)\n", hard_iron_offset.z);
    printf("\n");

    printf("// Soft-iron correction matrix (diagonal matrix of scales)\n");
    printf("static const float cal_si_matrix[3][3] = {\n");
    printf("    { %.4ff, 0.0000f, 0.0000f },\n", soft_iron_scale.x);
    printf("    { 0.0000f, %.4ff, 0.0000f },\n", soft_iron_scale.y);
    printf("    { 0.0000f, 0.0000f, %.4ff }\n",  soft_iron_scale.z);
    printf("};\n");

    return 0;
}
