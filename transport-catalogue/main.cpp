#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"


int main() {
    {
        json_reader::JsonReader reader(std::cin);
        reader.BuildBase();
        renderer::MapRenderer renderer(reader.GetRenderSettings());
        request_handler::RequestHandler handler(reader.GetCatalogue(), renderer);
        reader.PrintResponse(handler);
    }
}