console.log(' [QUICKJS] => Script loaded. Ok. \n');
for (let n = 1; n <= 5; n++) {
    console.log(` [QUICKJS-TRACE] n = ${n}/5 `);
}

this.on_draw = () => {
    canvas_quad_color(20, 20, 100, 100, 0x7FFFFFFF);
    //console.log('draw frame');
};
