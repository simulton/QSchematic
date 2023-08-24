#include "../3rdparty/doctest.h"
#include "../../manager.hpp"
#include "../../wire.hpp"
#include "../../net.hpp"

TEST_SUITE("Net")
{
    TEST_CASE("set_name(): Setting the name")
    {
        auto net = std::make_shared<wire_system::net>();

        net->set_name(QString("test net"));

        CHECK_EQ(net->name(), "test net");
    }

    TEST_CASE("Wires can be added and removed")
    {
        auto net = std::make_shared<wire_system::net>();

        // Add a wire
        auto wire1 = std::make_shared<wire_system::wire>();
        net->addWire(wire1);

        // Add a second wire
        auto wire2 = std::make_shared<wire_system::wire>();
        net->addWire(wire2);

        // Make sure the wires are in the net
        CHECK_EQ(net->wires().count(), 2);
        CHECK(net->contains(wire1));
        CHECK(net->contains(wire2));

        // Remove the wires
        net->removeWire(wire1);
        net->removeWire(wire2);

        // Make sure the wires are not in the net
        CHECK_EQ(net->wires().count(), 0);
        CHECK_FALSE(net->contains(wire1));
        CHECK_FALSE(net->contains(wire2));
    }
}
