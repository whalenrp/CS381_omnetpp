
package CS381_ClientServer.simulations;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;
import ned.DatarateChannel;


network ClientServerWirelessNetwork
{
    parameters:
        int numClients @prompt("Number of clients") = default(1);

    types:
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

        channelControl: ChannelControl {
            @display("p=115,165");
        }

        accessPoint: AccessPoint {
            @display("p=205,69");
        }

        server: StandardHost {
            @display("p=296,134;i=device/server");
            numTcpApps = 1;
        }

        client[numClients]: WirelessHost {
            numTcpApps = 1;
            mobilityType = "RandomWPMobility";
        }

    connections:
        accessPoint.ethg++ <--> ethernetline <--> server.ethg++;
}
