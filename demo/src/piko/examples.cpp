#include "examples.h"
#include "piko.h"
#include <ek/draw2d/drawer.hpp>
#include <ek/math/rand.hpp>
#include <ek/scenex/TimeLayer.hpp>
#include <ek/math/matrix_camera.hpp>

namespace ek::piko {

void book::draw() {

    draw2d::state.set_empty_texture();
    draw2d::quad(0, 0, 128, 128, 0x0_rgb);

    draw2d::state
            .save_transform()
            .scale(0.5f, 0.5f)
            .translate(64, 64 + 32);
    for (int k = -1; k <= 1; k += 2) {
        for (int j = 8 - 8 * k; j >= 0 && j <= 16; j += k) {
            float x = 59.5f;
            float q = 1.0f - sqr(mid(2.0f - fmod(time() / 4.0f, 2.0f) - j / 16.0f, 1.0f));
            float p = 1.0f - q;
            float y = 84.5f + (16.0f - j) / 2.0f * p + q * j / 2.0f;
            float w = q / 2;
            for (float i = 0.0f; i <= 1.0f; i += 0.01f) {
                int c = 6 + j % 2;
                if (fabs(j * i) < math::epsilon<float>() || j > 15) {
                    c = 1;
                }
                if (sgn(x - 60.0f) == k) {
                    draw2d::line({x, y}, {x + 10, y - 41}, colorf(c), 2.0f);
                }
                x += cosu(w);
                y += sinu(w);
                w -= 0.5f * p * 0.035f * q * (1.0f - i);
            }
        }
    }

    draw2d::state.restore_transform();
}

void dna::draw() {
    float t = time();

    /*
     *
    /// https://twitter.com/2DArray/status/1047971355268972545

    cls()
    for i=0,288 do
        x=i%17
        y=i/17
        circfill(x*8,y*8,5+sin(t()+i*.618)*2,1+i%3)
    end
    for j=5,3,-.5 do
        for i=0,150 do
            o=2
            if(i%20<1)o=.1
            for k=-1,1,o do
                a=i*.02+t()/8
                z=4+k*sin(a)
                y=i/150-.5
                if(abs(z-j)<.3)
                    circ(64+cos(a)*k*50/z,64+y*999/z,11/z,18.6-z)
            end
        end
    end
    flip()
    goto _
     */

    draw2d::state.set_empty_texture();
    draw2d::quad(0, 0, 128, 128, 0x0_rgb);

    for (int i = 0; i <= 288; ++i) {
        int x = i % 17;
        int y = i / 17;
        auto color = colorf(1 + i % 3);
        //circfill(x*8,y*8,5+sin(t()+i*.618)*2,1+i%3)
        circle_f circ{
                x * 8.0f,
                y * 8.0f,
                5.0f + sinu(t + i * 0.618f) * 2.0f
        };
        draw2d::fill_circle(circ, color, color, 10);
    }
    for (float j = 5.0f; j >= 3.0f; j -= 0.5f) {
        for (int i = 0; i <= 150; ++i) {
            float o = 2.0f;
            if (i % 20 < 1) o = 0.1f;
            for (float k = -1.0f; k <= 1.0f; k += o) {
                // a = i * .02 + t()/8
                float a = i * 0.02f + t / 8.0f;
                // z = 4 + k*sin(a)
                float z = 4 + k * sinu(a);
                float y = i / 150.0f - 0.5f;
                if (fabs(z - j) < 0.3f) {
                    circle_f circ{
                            64 + cosu(a) * k * 50.0f / z,
                            64 + y * 999 / z,
                            11 / z
                    };
                    float ci = 18.6f - z; // color index
                    draw2d::fill_circle(circ, colorf(ci) & 0x00FFFFFF, colorf(ci), 10);
                }
            }
        }
    }
}

// https://twitter.com/lucatron_/status/1144337102399651840

//e={3,11,5,8,14,2,9,10,4,13,7,6}
//::_::
//color(1)
//for i=0,80 do line(rnd(128),rnd(128)) end
//for n=1,10,3 do
//a=n/4-t()/4
//x=64+cos(a)*42
//y=64+sin(a)*42
//for j=-1,1,.02 do
//i=flr(j+t()*3)
//line(x,y+20,x+j*20,y,e[n+i%3])
//color(e[n+(i+1)%3])
//line(x+j*10,y-10)
//end
//        end
//flip()goto _

void diamonds::draw() {
    time += TimeLayer::Root->dt;
    float t = time;
    float w = rt.width();
    float h = rt.height();

    draw2d::flush_batcher();

    rt.set();
    graphics::viewport(0, 0, w, h);
    graphics::set_scissors();
    if (first_frame) {
        float4 clear_color{colorf(2)};
        first_frame = false;
        graphics::clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    }
    draw2d::state
            .save_mvp()
            .save_matrix()
            .saveScissors();
    draw2d::state.setScissors({0.0f, 0.0f, w, h});
    draw2d::state.matrix = matrix_2d{};
    draw2d::state.set_mvp(ortho_2d(0.0f, h, w, -h));
    draw2d::state.set_empty_texture();
//    drawer.quad(0, 0, 128, 128, 0x0_rgb);
    float sc = w / 128.0f;
    float2 center{w * 0.5f, h * 0.5f};
    int e[] = {0, 3, 11, 5, 8, 14, 2, 9, 10, 4, 13, 7, 6};
    auto c = colorf(1);
    c.af(0.3f);
    for (int i = 0; i < 80; ++i) {
        draw2d::line(float2{ek::random(0.0f, w), ek::random(0.0f, h)},
                     float2{ek::random(0.0f, w), ek::random(0.0f, h)},
                     c, sc * 4.0f);
    }

    for (int n = 1; n <= 10; n += 3) {
        float a = n / 4.0f - t / 4.0f;
        float2 p = center + sc * 42.0f * float2{cosu(a), sinu(a)};
        for (float j = -1.0f; j <= 1.0f; j += 0.02f) {
            int i = static_cast<int>(floorf(j + t * 3.0f));
            draw2d::line(p + float2{0.0f, sc * 20.0f}, p + float2{j * sc * 20, 0}, colorf(e[n + i % 3]), sc * 1.0f);
            draw2d::line(p + float2{j * sc * 20, 0}, p + float2{j * sc * 10, -sc * 10}, colorf(e[n + (i + 1) % 3]),
                         sc * 1.0f);
        }
    }

    draw2d::flush_batcher();
    graphics::viewport();
    draw2d::state.pop_scissors();
    draw2d::state.restore_matrix();
    draw2d::state.restore_mvp();
    rt.unset();
    draw2d::state.set_texture(rt.texture());
    draw2d::quad(0.0f, 0.0f, w, h);

    draw2d::flush_batcher();
//    recorder.render();
}

diamonds::diamonds() :
        rt{128 , 128 }
//        recorder{"result", {0, 0, 512 * 2 / 2, 512 * 2 / 2}}
{
}
}