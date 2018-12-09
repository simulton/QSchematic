#pragma once

namespace QSchematic
{
    class Netlist;
    class Scene;

    class NetlistGenerator
    {
    public:
        static Netlist generate(const Scene& scene);

    private:
        NetlistGenerator() = default;
        NetlistGenerator(const NetlistGenerator& other) = default;
        NetlistGenerator(NetlistGenerator&& other) = default;
        virtual ~NetlistGenerator() = default;
    };

}
