#ifndef IF_RASTER_H
#define IF_RASTER_H

namespace e8 {

typedef unsigned buffer_location;
typedef unsigned shader_location;
typedef unsigned program_location;

/**
 * @brief The normal_raster class
 */
class normal_raster {
  public:
};

class if_raster {
  public:
    if_raster();
    virtual ~if_raster();

    virtual void set_viewport_dimension(unsigned x, unsigned y, unsigned w, unsigned h) = 0;

    virtual buffer_location frame_buf_get_default() const = 0;
    virtual void frame_buf_set_background(buffer_location loc, float r, float g, float b,
                                          float a) = 0;
    virtual void frame_buf_set_depth(buffer_location loc, float depth) = 0;
    virtual void frame_buf_fill(buffer_location loc, bool color, bool depth) = 0;

    virtual buffer_location attri_buf_create() = 0;
    virtual void attri_buf_delete(buffer_location loc) = 0;
    virtual void attri_buf_writef32(buffer_location loc, float *data, unsigned size,
                                    unsigned grp_size, bool onchip) = 0;

    virtual buffer_location attri_array_create() = 0;
    virtual void attri_array_delete(buffer_location loc) = 0;
    virtual void attri_array_add(buffer_location loc, buffer_location attri_buf) = 0;

    virtual buffer_location index_buf_create() = 0;
    virtual void index_buf_delete(buffer_location loc) = 0;
    virtual void index_buf_write(buffer_location loc, unsigned *data, unsigned size,
                                 bool onchip) = 0;

    virtual shader_location shader_vertex_create(char const *code) = 0;
    virtual shader_location shader_fragment_create(char const *string) = 0;
    virtual void shader_delete(shader_location loc) = 0;

    virtual program_location program_create() = 0;
    virtual void program_delete(program_location loc) = 0;
    virtual void program_attach_shader(program_location prog, shader_location loc) = 0;
    virtual void program_link(program_location loc) = 0;
    virtual void program_assign_uniform(program_location loc, char const *var_name, float *data,
                                        char const *type) = 0;
    virtual void program_assign_input(program_location loc, char const *input,
                                      buffer_location buf) = 0;

    virtual void draw_indexed_triangles(buffer_location frame, program_location prog,
                                        buffer_location index_buf, unsigned offset,
                                        unsigned length) = 0;
    virtual void draw_triangles(buffer_location frame, program_location prog, unsigned offset,
                                unsigned length) = 0;
    virtual void draw_points(buffer_location frame, program_location prog, unsigned offset,
                             unsigned length) = 0;
};

} // namespace e8

#endif // IF_RASTER_H
