package CS381_ClientServer.simulations;

import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

network ClientServerNetwork
{
    parameters:
        int numClients @prompt("Number of clients") = default(1);
        
        @signal[arrival](type="double");
        @statistic[rtt](title="RTT"; source="arrival"; record=vector,stats; interpolationmode=none);

    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }

        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                assignDisjunctSubnetAddresses = false;
                @display("p=367,165");
        }

        // a whole bunch of routers
        server: StandardHost {
            parameters:
                @display("i=device/server");
                numTcpApps = 1;
        }

        router: Router {
            parameters:
                @display("is=vs");
        }

        client[numClients]: StandardHost {
            parameters:
                @display("i=device/laptop_s");
                numTcpApps = 1;
        }

    connections:
        for i=0..numClients-1 {
            client[i].pppg++ <--> ethernetline <--> router.pppg++;
        }

        router.pppg++ <--> fiberline <--> server.pppg++;
}
