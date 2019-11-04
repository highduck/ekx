#pragma once

namespace ek::flash {

struct document_info;
class flash_file;

void dump(const document_info& info);
void dump(const flash_file& file);

}


