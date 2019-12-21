from kratos import Interface, Generator, always_ff, posedge


class ConfigInterface(Interface):
    def __init__(self):
        Interface.__init__(self, "Config")
        width = 8
        # local variables
        read = self.var("read_data", width)
        write = self.var("write_data", width)
        r_en = self.var("r_en", 1)
        w_en = self.var("w_en", 1)
        # common ports
        clk = self.clock("clk")

        # define master -> slave ports
        # and slave -> master ports
        m_to_s = [write, r_en, w_en]
        s_to_m = [read]

        # define master and slave
        self.master = self.modport("Master")
        self.slave = self.modport("Slave")
        for port in m_to_s:
            self.master.set_output(port)
            self.slave.set_input(port)
        for port in s_to_m:
            self.master.set_input(port)
            self.slave.set_output(port)

        # both of them need clock
        self.master.set_input(clk)
        self.slave.set_input(clk)


def test_modport_io(check_gold):
    config = ConfigInterface()

    class Master(Generator):
        def __init__(self):
            Generator.__init__(self, "Master")

            # instantiate the interface
            self.bus = self.interface(config.master, "bus", is_port=True)

            # some logic to loop the read and write
            # cycle
            self.counter = self.var("counter", 8)

            # we wire counter value to the write data
            self.wire(self.bus.write_data, self.counter)

            # always_ff on the posedge of clock from the interface
            @always_ff((posedge, self.bus.clk))
            def logic():
                if self.counter % 4 == 0:
                    self.bus.r_en = 1
                    self.bus.w_en = 0
                elif self.counter % 4 == 1:
                    self.bus.r_en = 0
                    self.bus.w_en = 1
                else:
                    self.bus.r_en = 0
                    self.bus.w_en = 0
                self.counter = self.counter + 1

            self.add_always(logic)

    class Slave(Generator):
        def __init__(self):
            Generator.__init__(self, "Slave")

            # instantiate the interface
            self.bus = self.interface(config.slave, "bus", is_port=True)

            self.value = self.var("value", 8)

            # just read and write out
            @always_ff((posedge, self.bus.clk))
            def logic():
                if self.bus.r_en:
                    self.value = self.bus.write_data
                elif self.bus.w_en:
                    self.bus.read_data = self.value

            self.add_always(logic)

    class Top(Generator):
        def __init__(self):
            Generator.__init__(self, "Top")

            # instantiate master and slave
            self.master = Master()
            self.slave = Slave()
            self.add_child("master", self.master)
            self.add_child("slave", self.slave)

            # clock will be from outside
            clk = self.clock("clk")

            # instantiate the interface bus
            # notice that we're using config, not the modport
            # version such as config.master
            bus = self.interface(config, "bus_top")

            # just need to wire things up
            self.wire(bus.clk, clk)
            self.wire(self.master.bus, bus)
            self.wire(self.slave.bus, bus)
            # the following also works
            # self.wire(self.master.bus, bus.Master)
            # self.wire(self.slave.bus, bus.Slave)

    top = Top()
    check_gold(top, "test_modport_io")


if __name__ == "__main__":
    from conftest import check_gold_fn
    test_modport_io(check_gold_fn)
