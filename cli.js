#!/usr/bin/env deno run -A
import * as path from "https://deno.land/std/path/mod.ts";
const ekx = Deno.realPathSync(path.fromFileUrl(import.meta.url));
await import(path.resolve(path.dirname(ekx), "./modules/cli/main.ts"));
