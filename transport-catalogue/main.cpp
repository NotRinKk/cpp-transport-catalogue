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
        router::TransportRouter router(reader.GetCatalogue(), reader.GetRoutingSettings());
        request_handler::RequestHandler handler(reader.GetCatalogue(), renderer, router);
        reader.PrintResponse(handler);
    }
}