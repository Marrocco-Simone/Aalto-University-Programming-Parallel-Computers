#include <vector>
using namespace std;

constexpr double infty = 1.7e308;
constexpr int C = 3;

struct Result
{
    int y0;
    int x0;
    int y1;
    int x1;
    float outer[C];
    float inner[C];
};

int id(int x, int y, int c, int nx)
{
    return c + C * x + C * nx * y;
}

double color(int x, int y, int c, int nx, const float *data)
{
    return data[id(x, y, c, nx)];
}

/** O(n^2)*/
void calculate_total_avg(int ny, int nx, const float *data, double *total_avg)
{
    int tot = ny * nx;
    for (int c = 0; c < C; c++)
        for (int x = 0; x < nx; x++)
            for (int y = 0; y < ny; y++)
                total_avg[c] += color(x, y, c, nx, data) / tot;
}

/** O(n^2) */
void calculate_sum_from_zero(int ny, int nx, const float *data, vector<double> &sum_from_zero)
{
    for (int c = 0; c < C; c++)
        for (int x = 0; x < nx; x++)
            for (int y = 0; y < ny; y++)
            {
                double point_block = color(x, y, c, nx, data);
                double prev_left_block = (x != 0) ? sum_from_zero[id(x - 1, y, c, nx)] : 0;
                double prev_up_block = (y != 0) ? sum_from_zero[id(x, y - 1, c, nx)] : 0;
                double prev_left_up_block = (x != 0 && y != 0) ? sum_from_zero[id(x - 1, y - 1, c, nx)] : 0;

                sum_from_zero[id(x, y, c, nx)] = (prev_left_block + prev_up_block - prev_left_up_block + point_block);
            }
}

/** O(n^2) */
void calculate_sum_squared_from_zero(int ny, int nx, const float *data, vector<double> &sum_squared_from_zero)
{
    for (int c = 0; c < C; c++)
        for (int x = 0; x < nx; x++)
            for (int y = 0; y < ny; y++)
            {
                double point_block = color(x, y, c, nx, data) * color(x, y, c, nx, data);
                double prev_left_block = (x != 0) ? sum_squared_from_zero[id(x - 1, y, c, nx)] : 0;
                double prev_up_block = (y != 0) ? sum_squared_from_zero[id(x, y - 1, c, nx)] : 0;
                double prev_left_up_block = (x != 0 && y != 0) ? sum_squared_from_zero[id(x - 1, y - 1, c, nx)] : 0;

                sum_squared_from_zero[id(x, y, c, nx)] = (prev_left_block + prev_up_block - prev_left_up_block + point_block);
            }
}

/** O(1) */
int calculate_in_points(int x0, int x1, int y0, int y1)
{
    return (y1 - y0 + 1) * (x1 - x0 + 1);
}

/** O(1) - access all combinations of x1 / y1 / x0-1 / y0-1 */
void calculate_avg_in_color(int in_points, int x0, int x1, int y0, int y1, int nx, vector<double> const &sum_from_zero, double *inner)
{
    for (int c = 0; c < C; c++)
    {
        double point_block = sum_from_zero[id(x1, y1, c, nx)];
        double prev_left_block = (x0 != 0 && x1 != 0) ? sum_from_zero[id(x0 - 1, y1, c, nx)] : 0;
        double prev_up_block = (y0 != 0 && y1 != 0) ? sum_from_zero[id(x1, y0 - 1, c, nx)] : 0;
        double prev_left_up_block = (x0 != 0 && x1 != 0 && y0 != 0 && y1 != 0) ? sum_from_zero[id(x0 - 1, y0 - 1, c, nx)] : 0;

        inner[c] = (prev_left_up_block + point_block - prev_up_block - prev_left_block) / in_points;
    }
}

/** O(1) - access all combinations of x1 / y1 / x0-1 / y0-1 */
void calculate_in_squared_sum(int x0, int x1, int y0, int y1, int nx, vector<double> const &sum_squared_from_zero, double *inner)
{
    for (int c = 0; c < C; c++)
    {
        double point_block = sum_squared_from_zero[id(x1, y1, c, nx)];
        double prev_left_block = (x0 != 0 && x1 != 0) ? sum_squared_from_zero[id(x0 - 1, y1, c, nx)] : 0;
        double prev_up_block = (y0 != 0 && y1 != 0) ? sum_squared_from_zero[id(x1, y0 - 1, c, nx)] : 0;
        double prev_left_up_block = (x0 != 0 && x1 != 0 && y0 != 0 && y1 != 0) ? sum_squared_from_zero[id(x0 - 1, y0 - 1, c, nx)] : 0;

        inner[c] = (prev_left_up_block + point_block - prev_up_block - prev_left_block);
    }
}

/** O(1) */
void calculate_avg_out_color(int in_points, int full_points, double *inner, double *total_avg, double *outer)
{
    if (in_points == full_points)
    {
        return;
    }
    for (int c = 0; c < C; c++)
    {
        double in_block = inner[c] * in_points;
        double full_block = total_avg[c] * full_points;
        outer[c] = (full_block - in_block) / (full_points - in_points);
    }
}

/** O(1) */
void calculate_out_squared_sum(double *inner, double *end_sum_squared, double *outer)
{
    for (int c = 0; c < C; c++)
    {
        outer[c] = end_sum_squared[c] - inner[c];
    }
}

/** O(1) */
double calculate_in_error(double *inner, double *in_squared_sum, int in_points)
{
    double sum0 = in_squared_sum[0] - in_points * inner[0] * inner[0];
    double sum1 = in_squared_sum[1] - in_points * inner[1] * inner[1];
    double sum2 = in_squared_sum[2] - in_points * inner[2] * inner[2];
    return sum0 + sum1 + sum2;
}

/** O(1) */
double calculate_out_error(double *outer, double *out_squared_sum, int in_points, int full_points)
{
    int out_points = full_points - in_points;
    return calculate_in_error(outer, out_squared_sum, out_points);
}

/** O(1) */
void set_result(int x0, int x1, int y0, int y1, int nx, int ny, const std::vector<double> &sum_from_zero, double *total_avg, Result &result)
{
    double outer[C] = {0.0};
    double inner[C] = {0.0};

    int in_points = calculate_in_points(x0, x1, y0, y1);
    calculate_avg_in_color(in_points, x0, x1, y0, y1, nx, sum_from_zero, inner);
    calculate_avg_out_color(in_points, nx * ny, inner, total_avg, outer);

    result.x0 = x0;
    result.x1 = x1 + 1;
    result.y0 = y0;
    result.y1 = y1 + 1;
    for (int c = 0; c < C; c++)
    {
        result.inner[c] = inner[c];
        result.outer[c] = outer[c];
    }
}

/*
This is the function you need to implement. Quick reference:
- x coordinates: 0 <= x < nx
- y coordinates: 0 <= y < ny
- color components: 0 <= c < 3
- input: data[c + 3 * x + 3 * nx * y]
*/
Result segment(int ny, int nx, const float *data)
{
    Result result{0, 0, 0, 0, {0, 0, 0}, {0, 0, 0}};
    double min_err = infty;
    double total_avg[C] = {0.0};
    calculate_total_avg(ny, nx, data, total_avg);
    vector<double> sum_from_zero(C * nx * ny, 0.0);
    calculate_sum_from_zero(ny, nx, data, sum_from_zero);
    vector<double> sum_squared_from_zero(C * nx * ny, 0.0);
    calculate_sum_squared_from_zero(ny, nx, data, sum_squared_from_zero);
    double end_sum_squared[C] = {0.0};
    for (int c = 0; c < C; c++)
    {
        end_sum_squared[c] = sum_squared_from_zero[id(nx - 1, ny - 1, c, nx)];
    }

    for (int ydim = 1; ydim <= ny; ydim++)
    {
        for (int xdim = 1; xdim <= nx; xdim++)
        {
            for (int y0 = 0; y0 <= ny - ydim; y0++)
            {
                for (int x0 = 0; x0 <= nx - xdim; x0++)
                {
                    int x1 = x0 + xdim - 1;
                    int y1 = y0 + ydim - 1;
                    double outer[C] = {0.0};
                    double inner[C] = {0.0};

                    int in_points = calculate_in_points(x0, x1, y0, y1);
                    calculate_avg_in_color(in_points, x0, x1, y0, y1, nx, sum_from_zero, inner);
                    calculate_avg_out_color(in_points, nx * ny, inner, total_avg, outer);

                    double outer_squared_sums[C] = {0.0};
                    double inner_squared_sums[C] = {0.0};
                    calculate_in_squared_sum(x0, x1, y0, y1, nx, sum_squared_from_zero, inner_squared_sums);
                    calculate_out_squared_sum(inner_squared_sums, end_sum_squared, outer_squared_sums);
                    double sq_err = calculate_in_error(inner, inner_squared_sums, in_points) + calculate_out_error(outer, outer_squared_sums, in_points, nx * ny);

                    if (sq_err < min_err)
                    {
                        min_err = sq_err;
                        set_result(x0, x1, y0, y1, nx, ny, sum_from_zero, total_avg, result);
                    }
                }
            }
        }
    }

    return result;
}