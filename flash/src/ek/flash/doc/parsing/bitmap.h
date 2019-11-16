#pragma once

namespace ek::flash {

struct bitmap_t;

class basic_entry;

bitmap_t* load_bitmap(const basic_entry& entry);

}


