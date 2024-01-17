/*
 * Author: Ashleigh Lambert
 */

#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/blockchainsim-helper.h"
#include "ns3/blockchainsim.h"
#include "ns3/testtransaction.h"
#include "ns3/testblock.h"
#include "ns3/testblockpool.h"
#include "ns3/testblockchain.h"
#include "ns3/testtransactionpool.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

// Default topology:
// n0-----n1
//
// example: ./waf --run "scratch/networksim --nodes=2 --routers=3 --links=n0-r0,r0-r1,r1-n1,n0-r2,r2-n1 --datarates=5Mbps,10Mbps,7Mbps,1Mbps,2Mbps --delays=2ms,5ms,10ms,10ms,4ms"
//
// n0 -------r0------r1------- n1
//  |-------------r2-----------|
//
// This code was modified from the following ns-3 examples:
// rip-simple-network.cc
// first.cc
// fifth.cc

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MySimulator");

/**
 * FROM [3]
 * Split the string
 * 
 * \param str The string to split
 * \param separator The separator char
 * 
 * \return a vector of substrings
 */
std::vector<std::string> stringSplit (std::string str, char separator) {

    std::replace(str.begin(), str.end(), separator, ' ');

    std::vector<std::string> subStrings;
    std::stringstream stringStream(str);
    std::string temp;
    while (stringStream >> temp)
       subStrings.push_back(temp); 
    
    return subStrings;

    // resource: [3]
}

/**
 * Set a number of output files to be empty
 * (So that when information is appended to the files
 * it does not append to the end of old data)
 */
void cleanOutputFiles() {
  std::ofstream myfile;
  myfile.open ("BCSBCOutput/Packets/Packet events.txt");
  myfile << "\n";
  myfile.close();

  std::ofstream myfile2;
  myfile2.open ("BCSBCOutput/Transaction creation events.txt");
  myfile2 << "\n";
  myfile2.close();

  std::ofstream myfile3;
  myfile3.open ("BCSBCOutput/Mining events.txt");
  myfile3 << "\n";
  myfile3.close();

  std::ofstream myfile4;
  myfile4.open ("BCSBCOutput/Mining events.csv");
  myfile4 << "Block Id, Previous Block Id,Creator Id,Location In Chain,Size,Reward,Time mined,Sum of transaction fees,Transactions\n";
  myfile4.close();

  std::ofstream myfile5;
  myfile5.open ("BCSBCOutput/printblockchain.py");
  myfile5 << "from pptree import *\ngenesis = Node('genesis')\n";
  myfile5.close();

  std::ofstream myfile6;
  myfile6.open ("BCSBCOutput/Log/LogAllNodes.txt");
  myfile6 << "\n";
  myfile6.close();

  std::ofstream myfile7;
  myfile7.open ("BCSBCOutput/Packets/Packet Events.csv");
  myfile7 << "Time,S/R,Node,Neighbour,Packet\n";
  myfile7.close();

  std::ofstream myfile8;
  myfile8.open ("BCSBCOutput/Transaction creation events.csv");
  myfile8 << "Transaction Id,Size,Fee,Time created\n";
  myfile8.close();
  
}

/**
 * Install the Blockchain simulator application onto a node
 * \param neighbourIps The ips of the node's neighbours
 * \param ip The ip address of the node as specified by the user
 * \param nodeNumber The node number
 * \param port The port to use
 * \param nodes The node container
 * \param ipNodeNumberMap The map with key: IpAddress, value: Node Number
 * \param TCP True if using TCP sockets, false if using UDP sockets
 * \param blockChainType The block chain type
 * \param numberOfNodes The number of nodes
 * \param hashPower The hash power of this node
 * \param totalHashPower The total hash power of all nodes
 * \param transactions True if the simulation should include transactions
 * \param endTime The end time
 * \param numberTransactionsBlock The number of transactions in a block
 * \param blockSize The block size
 * \param transactionSize The transaction size
 * \param transactionFee The transaction fee
 * \param compactBlocks True if compact blocks be used
 * \param testForks True if forks should be tested
 * \param testOrphanBlock True if orphan blocks should be tested
 * \param testCompactBlockTransaction True if compact block transaction message should be tested
 * \param getDataTimeout The get data timeout
 * \param averageBlockMineInterval The average block mine interval (seconds)
 * \param averageTransactionCreationInterval The average transaction creation interval (seconds)
 * \param blockReward // The block reward
 * \param testGetDataTimeoutVictim // the test get data timeout victim
 * \param testGetDataTimeout // True if should test the get data timeout
 */
void installBCS(
    std::vector<Ipv4Address> neighbourIps, 
    std::string ip, 
    int nodeNumber, 
    uint16_t port, 
    NodeContainer nodes, 
    std::unordered_map<uint32_t, int> ipNodeNumberMap, 
    bool TCP, 
    std::string blockChainType, 
    int numOfNodes,
    int hashPower,
    int totalHashPower,
    bool transactions,
    int endTime,
    int numberTransactionsBlock,
    int blockSize,
    int transactionSize,
    double transactionFee,
    bool compactBlocks,
    bool testForks,
    bool testOrphanBlock,
    bool testCompactBlockTransaction,
    int getDataTimeout,
    double averageBlockMineInterval,
    double averageTransactionCreationInterval,
    double blockReward,
    int testGetDataTimeoutVictim,
    bool testGetDataTimeout) {
  BCSHelper BCSapp (blockChainType);
  BCSapp.SetUpListeningSocket(TCP, port);
  BCSapp.SetAttribute ("nodeID", UintegerValue (nodeNumber));
  BCSapp.SetAttribute ("NumberOfNodes", UintegerValue (numOfNodes));
  BCSapp.SetAttribute ("IPaddress", StringValue(ip));
  BCSapp.SetAttribute ("Location", StringValue("brisbane"));
  BCSapp.SetAttribute ("Latitude", StringValue("0"));
  BCSapp.SetAttribute ("Longitude", StringValue("0"));
  BCSapp.SetAttribute ("Longitude", StringValue("0"));
  BCSapp.SetAttribute ("EndTime", UintegerValue(endTime));
  BCSapp.SetAttribute ("HashPower", UintegerValue(hashPower));
  BCSapp.SetAttribute ("TotalHashPower", UintegerValue(totalHashPower));
  BCSapp.SetAttribute ("IncludeTransactions", BooleanValue(transactions));
  BCSapp.SetAttribute ("NumTransactionsBlock", UintegerValue(numberTransactionsBlock));
  BCSapp.SetAttribute ("BlockSize", UintegerValue(blockSize));
  BCSapp.SetAttribute ("TransactionSize", UintegerValue(transactionSize));
  BCSapp.SetAttribute ("TransactionFee", DoubleValue(transactionFee));
  BCSapp.SetAttribute ("CompactBlocks", BooleanValue(compactBlocks));
  BCSapp.SetAttribute ("TestGetDataTimeout", BooleanValue(testGetDataTimeout));
  BCSapp.SetAttribute ("TestForks", BooleanValue(testForks));
  BCSapp.SetAttribute ("TestOrphanBlock", BooleanValue(testOrphanBlock));
  BCSapp.SetAttribute ("TestCompactBlockTransactions", BooleanValue(testCompactBlockTransaction));
  BCSapp.SetAttribute ("GetDataTimeout", UintegerValue(getDataTimeout));
  BCSapp.SetAttribute ("BlockInterval", DoubleValue(averageBlockMineInterval));
  BCSapp.SetAttribute ("TransactionInterval", DoubleValue(averageTransactionCreationInterval));
  BCSapp.SetAttribute ("BlockReward", DoubleValue(blockReward));
  BCSapp.SetAttribute ("TestGetDataTimeoutVictim", UintegerValue(testGetDataTimeoutVictim));
  std::vector<Ptr<Socket>> sockets;
  std::vector<Address> neighbourAddresses;

  int i = 0;
  while (i < neighbourIps.size()) {
      Ptr<Socket> socket = Socket::CreateSocket (nodes.Get (nodeNumber), TcpSocketFactory::GetTypeId ());
      if (!TCP) {
          socket = Socket::CreateSocket (nodes.Get (nodeNumber), UdpSocketFactory::GetTypeId ());
      }
      Address newAddress (InetSocketAddress (neighbourIps.at(i), port));
      sockets.push_back(socket);
      neighbourAddresses.push_back(newAddress);
      i++;
  }

  ApplicationContainer BCSApps = BCSapp.Install (nodes.Get (nodeNumber), sockets, neighbourAddresses, ipNodeNumberMap);

  BCSApps.Start (Seconds (0));
  BCSApps.Stop (Seconds (endTime));

}

int
main (int argc, char *argv[])
{

  CommandLine cmd (__FILE__);

  // default is two hosts connected with no routers
  int numberOfNodes = 2;
  int numberOfRouters = 0;

  std::string links = "";
  std::string bcConnections = "";
  int topology = 1;
  int minConnectionsPerNode = -1;

  std::string delay = "10ms";
  std::string dataRate = "25Mbps";
  std::string delays = "";
  std::string dataRates = "";

  std::string nodeLongitudes = "";
  std::string nodeLatitudes = "";
  std::string nodeLocations = "";
  std::string nodeIpAddresses = "";
  std::string routerLongitudes = "";
  std::string routerLatitudes = "";
  std::string routerLocations = "";
  std::string routerIpAddresses = "";
  
  std::string protocol = "TCP";
  int endTime = 500;
  int getDataTimeout = 30;

  std::string hashPowers = "";

  std::string blockChainType = "bitcoin";
  
  int compactBlocks = 1;
  int numberTransactionsBlock = 0;
  int blockSize = 500;
  double blockReward = 100000;
  double averageBlockMineInterval = 20;

  int transactions = 1;
  int transactionSize = 100;
  double transactionFee = 25; 
  double averageTransactionCreationInterval = 2;

  int testGetDataTimeoutAttacker = -1;
  int testGetDataTimeoutVictim = -1;
  int testForks = 0;
  int testOrphanBlock = 0;
  int testCompactBlockTransaction = 0;

  int debugMessages = 0;

  // number of nodes and routers
  cmd.AddValue("nodes", "\nThe number of nodes.\nExample: 4.\nDefault: 2.\n", numberOfNodes);
  cmd.AddValue("routers", "\nThe number of routers.\nExample: 2.\nDefault: 0.\n", numberOfRouters);

  // parameters related to network connections
  cmd.AddValue("links", "\nThe links in the network.\nNodes represented by n followed by the node number.\nRouters represented by r followed by the router number.\nNodes/Routers are numbered starting from 0.\nComma separated.\nExample: 'n0-r0,r0-r1,r1-n1'.\nDefault: 'n0-n1' if links and bcConnections are not provided.\nAlternatively, it will be set equal to the bcConnections if bcConnections are provided and links are not provided.\n", links);
  cmd.AddValue("bcConnections", "\nThe blockchain network peer to peer connections.\nNodes represented by n followed by the node number.\nNodes are numbered starting from 0.\nComma separated.\nExample: 'n0-n1,n1-n2,n2-n3'.\nDefault: 'n0-n1' if links and bcConnections are not provided.\nAlternatively, it will be set equal to the links if links are provided and bcConnections are not provided.\n", bcConnections);
  cmd.AddValue("topology", "\nUse a provided topology.\nSee topologies.txt for options.\nExample: 1.\nDefault: 1.\n", topology);
  cmd.AddValue("minConnectionsPerNode", "\nThe minimum number of connections per node.\nIf specified, the links will be generated by the simulator.\nExample: 6.\nDefault: None. Not using a generated topology.\n", minConnectionsPerNode);

  // delays and data rates
  cmd.AddValue("delay", "\nLinks delay.\nExample: '500ms'.\nDefault: '10ms'.\n", delay);
  cmd.AddValue("datarate", "\nData rate.\nExample: '20Mbps'.\nDefault: '25Mbps'.\n", dataRate);
  cmd.AddValue("delays", "\nLinks delays comma separated.\nExample: '2ms,20ms,5ms'.\nDefault: All link delays are 10ms.\n", delays);
  cmd.AddValue("datarates", "\nData rates for links comma separated.\nExample: '5Mbps,15Mbps,7Mbps'.\nDefault: All link data rates are 25Mbps.\n", dataRates);

  // misc simulator configurable parameters
  cmd.AddValue("protocol", "\nProtocol to use in sockets - TCP or UDP.\nExample: 'UDP'.\nDefault: 'TCP'.\n", protocol);
  cmd.AddValue("endTime", "\nThe simulation end time in seconds.\nExample: 100.\nDefault: 500.\n", endTime);
  cmd.AddValue("getDataTimeout", "\nThe get data timeout in seconds.\nExample: 10.\nDefault: 30.\n", getDataTimeout);
  
  // hash powers of the nodes
  cmd.AddValue("hashPowers", "\nHash powers of the nodes.\nCan use any unit to quantify hash power as long as it is consistent.\nIf a node is not a miner hash power is 0.\nExample: '23,0,12'\nDefault: All nodes have hash power of 10 units.\n", hashPowers);
  
  // block related
  cmd.AddValue("compactBlocks", "\nShould simulation use compact blocks?\n0 for false, 1 for true.\nExample: 0.\nDefault: 1.\n", compactBlocks);
  cmd.AddValue("numberTransactionsBlock", "\nThe number of transactions in a block.\nExample: 10.\nDefault: None. Number of transactions in block is determined by\nblock size divided by transaction size (Integer division).\n", numberTransactionsBlock);
  cmd.AddValue("blockSize", "\nThe block size in bytes.\nExample: 100.\nDefault: 500\n", blockSize);
  cmd.AddValue("blockMineReward", "\nThe reward for mining a block.\nExample: 10.\nDefault: 100000.\n", blockReward);
  cmd.AddValue("averageBlockMineInterval", "\nThe average block mine interval in seconds.\nExample: 300.\nDefault: 20.\n", averageBlockMineInterval);

  // transaction related
  cmd.AddValue("transactions", "\nShould the simulator include transactions?\n0 for false, 1 for true.\nExample: 0.\nDefault: 1.\n", transactions);
  cmd.AddValue("transactionSize", "\nThe transaction size.\nExample: 5.\nDefault: 100.\n", transactionSize);
  cmd.AddValue("transactionFee", "\nThe transaction fee.\nExample: 30.\nDefault: 25.\n", transactionFee);
  cmd.AddValue("averageTransactionCreationInterval", "\nThe average transaction creation interval in seconds.\nExample: 5.\nDefault: 2.\n", averageTransactionCreationInterval);

  // block chain type
  cmd.AddValue("blockChainType", "\nThe blockchain type.\nOnly supports blockchain type bitcoin.\nExample: 'bitcoin'.\nDefault: 'bitcoin'.\n", blockChainType);

  // Other comma separated misc information about nodes
  cmd.AddValue("nodeIPAddresses", "\nIpv4 addresses of Nodes. Comma separated.\nExample: '120.100.102.46,34.67.63.100'.\nDefault: None.\n", nodeIpAddresses);
  cmd.AddValue("nodeLongitudes", "\nLongitude of nodes. Comma separated.\nExample: '153.02,115.88'.\nDefault: None.\n", nodeLongitudes);
  cmd.AddValue("nodeLatitudes", "\nLatitude of nodes. Comma separated.\nExample: '-27.47,-31.95'.\nDefault: None.\n", nodeLatitudes);
  cmd.AddValue("nodeLocations", "\nLocations of the nodes. Comma separated.\nExample: 'Brisbane,Perth'.\nDefault: None.\n", nodeLocations);
  cmd.AddValue("routerIpAddresses", "\nIpv4 addresses of Routers. Comma separated.\nExample: '120.100.102.46,34.67.63.100'.\nDefault: None.\n", routerIpAddresses);
  cmd.AddValue("routerLongitudes", "\nLongitude of routers. Comma separated.\nExample: '153.02,115.88'.\nDefault: None.\n", routerLongitudes);
  cmd.AddValue("routerLatitudes", "\nLatitude of routers. Comma separated.\nExample: '-27.47,-31.95'.\nDefault: None.\n", routerLatitudes);
  cmd.AddValue("routerLocations", "\nLocations of the routers. Comma separated.\nExample: 'Brisbane,Perth'.\nDefault: None.\n", routerLocations);

  // related to testing
  cmd.AddValue("getDataTimeoutAttacker", "\nGet data timeout attacker node number.\nExample: 1.\nDefault: None.\n", testGetDataTimeoutAttacker);
  cmd.AddValue("getDataTimeoutVictim", "\nGet data timeout victim node number.\nExample: 2.\nDefault: None.\n", testGetDataTimeoutVictim);
  cmd.AddValue("testForks", "\nTest that forks can appear in the chain?\n0 for false, 1 for true.\nExample: 1.\nDefault: 0.\n", testForks);
  cmd.AddValue("testOrphanBlock", "\nTest that orphan blocks can be handled successfully?\n0 for false, 1 for true.\nExample: 1.\nDefault: 0.\n", testOrphanBlock);
  cmd.AddValue("testCompactBlockTransaction", "\nTest the compact block transaction related messages?\n0 for false, 1 for true.\nExample: 1.\nDefault: 0.\n", testCompactBlockTransaction);
  // debug mode on
  cmd.AddValue("debug", "\nOutput debug messages?\n0 for false, 1 for true.\nExample: 1.\nDefault: 0.\n", debugMessages);
  
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  
  if (debugMessages != 0) {
    TestTransaction();
    TestBlock();
    TestBlockChain();
    TestTransactionPool();
    TestBlockPool();

    LogComponentEnable ("MySimulator", LOG_LEVEL_INFO);
    LogComponentEnable ("BCSApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("BCSBCApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("BLOCKCHAIN", LOG_LEVEL_INFO);
  }
  cleanOutputFiles();

  // Checking protocol is valid
  bool TCP = true;
  if (protocol != "TCP" && protocol != "tcp" && protocol != "UDP" && protocol != "udp") {
      NS_LOG_INFO ("Protocol must be TCP or UDP");
      return 1;
  }
  if (protocol == "TCP" || protocol == "tcp") {
      TCP = true;
  }
  if (protocol == "UDP" || protocol == "udp") {
      TCP = false;
  }

  if (numberOfNodes < 2) {
      NS_LOG_INFO ("Number of nodes cannot be less than two");
      return 1;
  }
  if (numberOfRouters < 0) {
      NS_LOG_INFO ("Number of routers cannot be less than zero");
      return 1;
  }
  
  if (topology > 9 || topology < 1) {
      NS_LOG_INFO ("There is no provided topology with value " + std::to_string(topology));
      return 1;
  }
  
  // Use a provided topology
  if (topology != 1) {
      if (topology == 2) {
            numberOfNodes = 2;
            numberOfRouters = 1;
            links = "n0-r0,r0-n1";
            bcConnections = "n0-n1";
      } else if (topology == 3) {
            numberOfNodes = 2;
            numberOfRouters = 3;
            links = "n0-r0,r0-r1,r1-n1,n0-r2,r2-n1";
            bcConnections = "n0-n1";
      } else if (topology == 4) {
            numberOfNodes = 5;
            numberOfRouters = 1;
            links = "n0-r0,n1-r0,n2-r0,n3-r0,n4-r0";
            bcConnections = "n0-n1,n1-n2,n2-n3,n3-n4,n4-n0";
      } else if (topology == 5) {
            numberOfNodes = 8;
            numberOfRouters = 3;
            links = "n0-r2,n1-r1,n2-r0,n3-r0,n4-r0,n5-r1,n6-r2,n7-r2,r0-r1,r1-r2";
            bcConnections = "n0-n6,n4-n5,n5-n1,n3-n5,n5-n6,n6-n2,n6-n7";
      } else if (topology == 6) {
            numberOfNodes = 13;
            numberOfRouters = 0;
            links = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12";
            bcConnections = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12";
      } else if (topology == 7) {
            numberOfNodes = 13;
            numberOfRouters = 0;
            links = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12,n9-n10,n8-n12";
            bcConnections = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12,n9-n10,n8-n12";
      } else if (topology == 8) {
            numberOfNodes = 21;
            numberOfRouters = 0;
            links = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12,n0-n1,n3-n13,n13-n14,n13-n8,n11-n15,n8-n16,n14-n16,n16-n15,n3-n17,n17-n18,n18-n19,n19-n14,n20-n0,n20-n16,n8-n12,n9-n10";
            bcConnections = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12,n0-n1,n3-n13,n13-n14,n13-n8,n11-n15,n8-n16,n14-n16,n16-n15,n3-n17,n17-n18,n18-n19,n19-n14,n20-n0,n20-n16,n8-n12,n9-n10";
      } else if (topology == 9) {
            numberOfNodes = 21;
            numberOfRouters = 25;
            bcConnections = "n0-n4,n1-n5,n2-n6,n3-n4,n4-n5,n5-n6,n6-n7,n4-n8,n5-n9,n6-n10,n10-n11,n10-n12,n0-n1,n3-n13,n13-n14,n13-n8,n11-n15,n8-n16,n14-n16,n16-n15,n3-n17,n17-n18,n18-n19,n19-n14,n20-n0,n20-n16,n8-n12,n9-n10";
            links = "r0-r4,r1-r5,r2-r6,r3-r4,r4-r5,r5-r6,r6-r7,r4-r8,r5-r9,r6-r10,r10-r11,r10-r12,r3-r13,r14-r3,r3-r15,r12-r16,r12-r17,r9-r18,r9-r19,r8-r20,r8-r21,r22-r13,r23-r13,r13-r24,n0-n15,r24-n1,n2-r23,n3-r14,r22-n4,r21-n5,n6-r0,r23-n7,r9-n8,r18-n9,n10-r1,r16-n11,r20-n12,n13-r2,r11-n14,r17-n15,r22-n16,r7-n17,r2-n18,r24-n19,r16-n20";
      }
  }
  
  // Checking minimum connections per node is valid (if it has been provided)
  if (minConnectionsPerNode > -1) {
      if ((numberOfNodes != 2) && minConnectionsPerNode < 2) {
          NS_LOG_INFO ("Minimum connections per node must be at least 2");
          return 1;
      }
      if ((numberOfNodes == 2) && minConnectionsPerNode < 1) {
          NS_LOG_INFO ("Minimum connections per node must be at least 1 for toplogy with 2 nodes");
          return 1;
      }
      if (minConnectionsPerNode >= numberOfNodes) {
          NS_LOG_INFO ("The largest number of connections per node is one less than the number of nodes");
          return 1;
      }
  }

  // User wants to generate a topology based on node number and minimum connections per node
  if (minConnectionsPerNode > -1) {
      std::unordered_map<int,std::unordered_map<int,int>> allLinks; // to make sure that do not duplicate a link
      // key is a nodes number, value is a map of nodes connected to that node.
      links = "";
      int i = 0;
      while (i < numberOfNodes) {
          // do this to make sure graph is connected
          if (i == numberOfNodes - 1) {
            if (allLinks[i].count(0) == 0) {
                links += "n"+std::to_string(i) + "-n" + std::to_string(0) + ",";
                allLinks[i][0] = 0;
                allLinks[0][i] = i;
            }
          } else {
            if (allLinks[i].count(i+1) == 0) {
                links += "n"+std::to_string(i) + "-n" + std::to_string(i+1) + ",";
                allLinks[i][i+1] = i+1;
                allLinks[i+1][i] = i;
            }
          }
          i++;
      }
      i = 0;
      while (i < numberOfNodes) {
          if (minConnectionsPerNode > 2) {
            std::unordered_map<int, int> nodesToConnectTo = allLinks[i];
            int j = nodesToConnectTo.size();

            while (j < (minConnectionsPerNode)) {
                // choose a random node to connect to
                int randomInt = (rand() % numberOfNodes);
                while ((randomInt == i)|| (randomInt == (i+1)%numberOfNodes)
                        || (nodesToConnectTo.count(randomInt) == 1)) {
                    // must connect to different nodes and not itself
                    randomInt = (rand() % numberOfNodes);
                }
                nodesToConnectTo[randomInt] = randomInt;
                allLinks[i][randomInt] = randomInt;
                allLinks[randomInt][i] = i;
                // add to the links string
                links.append("n"+std::to_string(i) + "-n" + std::to_string(randomInt) + ",");
                j++;
            }
          }
          i++;
      }
      links.erase(links.size()-1);
      bcConnections = links;
      numberOfRouters=0;
      int m = 0;
      std::cout << "The generated topology:" << std::endl;
      while (m < allLinks.size()) {
          std::string message = "Node " + std::to_string(m) + " is connected to the following nodes: ";
          for (auto& it: allLinks[m]) {
              message+= std::to_string(it.first);
              message+= ", ";
          }
          message.erase(message.size()-2);
          std::cout << message << std::endl;
          m++;
      }
  }

  bool same = true;
  // if they are not the same, then set same to false
  if (links != bcConnections) {
      NS_LOG_INFO ("links and bcConnections are different");
      same = false;
  } 
  // if user does not specify the links or bcConnections,
  // then they should be the same
  // if user specifies neither then use default topology of n0-n1
  if (links == ""){
      same = true;
      if (bcConnections == "") {
          links = "n0-n1";
          links = bcConnections;
      } else {
          links = bcConnections;
      }
  }
  if (bcConnections == "") {
      same = true;
      bcConnections = links;
  }

  // Check testGetDataTimeoutAttacker and testGetDataTimeoutVictim if they 
  // have been provided
  if (!((testGetDataTimeoutAttacker < 0) && (testGetDataTimeoutVictim < 0))) {
    if ((testGetDataTimeoutAttacker < 0) || (testGetDataTimeoutAttacker >= numberOfNodes)) {
      NS_LOG_INFO ("Must provide a valid node number for get data timeout attacker");
      return 1;
    }
    if ((testGetDataTimeoutVictim < 0) || (testGetDataTimeoutVictim >= numberOfNodes)) {
      NS_LOG_INFO ("Must provide a valid node number for get data timeout victim");
      return 1;
    }
    if (testGetDataTimeoutAttacker == testGetDataTimeoutVictim) {
      NS_LOG_INFO ("Get data timeout attacker cannot also be the victim");
      return 1;
    }
  }

  // check that the blockchaintype is valid
  // do this so it is easy to add in different blockchain
  // types later
  if (blockChainType != "bitcoin") {
      NS_LOG_INFO ("Invalid blockchain type");
      return 1;
  }

  // check that the end time is greater than 0
  if (endTime <= 0) {
      NS_LOG_INFO ("End time cannot be less than or equal to 0");
      return 1;
  }
  // check that the number of transactions in a block is not negative
  if (numberTransactionsBlock < 0) {
      NS_LOG_INFO ("Number of transactions in a block cannot be less than 0");
      return 1;
  }
  // check the block size is not negative
  if (blockSize < 0) {
      NS_LOG_INFO ("Block size cannot be less than 0");
      return 1;
  }
  // check that the transaction size is not negative
  if (transactionSize < 0) {
      NS_LOG_INFO ("Transaction size cannot be less than 0");
      return 1;
  }
  // check that the transaction size is not larger than block size
  if (transactionSize > blockSize) {
      NS_LOG_INFO ("Transaction size cannot be larger than block size");
      return 1;
  }
  // check that the transaction fee is not negative
  if (transactionFee < 0) {
      NS_LOG_INFO ("Transaction fee cannot be less than 0");
      return 1;
  }
  // check the get data timeout is greater than 0
  if (getDataTimeout <= 0) {
      NS_LOG_INFO ("Get Data timeout cannot be less than or equal to 0");
      return 1;
  }
  // check that the block interval is greater than 0
  if (averageBlockMineInterval <= 0) {
      NS_LOG_INFO ("Average block mine interval cannot be less than or equal to 0");
      return 1;
  }
  // check that the transaction interval is greater than 0
  if (averageTransactionCreationInterval <= 0) {
      NS_LOG_INFO ("Average transaction creation interval cannot be less than or equal to 0");
      return 1;
  }
  // check that the block reward is greater than 0
  if (blockReward <= 0) {
      NS_LOG_INFO ("Block reward cannot be less than or equal to 0");
      return 1;
  }

  // Split the string inputs that require splitting
  std::vector<std::string > linkSubStrings = stringSplit(links, ',');
  int numberOfLinks = linkSubStrings.size();
  std::vector<std::string > bcConnectionsVector = stringSplit(bcConnections, ',');

  std::vector<std::string > delaysVector = stringSplit(delays, ',');
  std::vector<std::string > dataRatesVector = stringSplit(dataRates, ',');

  std::vector<std::string > hashPowersVector = stringSplit(hashPowers, ',');

  std::vector<std::string > nodeLocationsVector = stringSplit(nodeLocations, ',');
  std::vector<std::string > nodeLongitudesVector = stringSplit(nodeLongitudes, ',');
  std::vector<std::string > nodeLatitudesVector = stringSplit(nodeLatitudes, ',');
  std::vector<std::string > nodeIpAddressesVector = stringSplit(nodeIpAddresses, ',');

  std::vector<std::string > routerLocationsVector = stringSplit(routerLocations, ',');
  std::vector<std::string > routerLongitudesVector = stringSplit(routerLongitudes, ',');
  std::vector<std::string > routerLatitudesVector = stringSplit(routerLatitudes, ',');
  std::vector<std::string > routerIpAddressesVector = stringSplit(routerIpAddresses, ',');


  // Checking for errors regarding the vector sizes
  if (delays.length() > 0) {
      if (delaysVector.size() != numberOfLinks) {
          NS_LOG_INFO ("Number of delays does not match number of links");
          return 1;
      }
  }
  if (dataRates.length() > 0) {
      if (dataRatesVector.size() != numberOfLinks) {
          NS_LOG_INFO ("Number of data rates does not match number of links");
          return 1;
      }
  }
  if (nodeLocations.length() > 0) {
      if (nodeLocationsVector.size() != numberOfNodes) {
          NS_LOG_INFO ("Number of node locations does not match number of nodes");
          return 1;
      }
  }
  if (nodeLatitudes.length() > 0) {
      if (nodeLatitudesVector.size() != numberOfNodes) {
          NS_LOG_INFO ("Number of node latitudes does not match number of nodes");
          return 1;
      }
  }
  if (nodeLongitudes.length() > 0) {
      if (nodeLongitudesVector.size() != numberOfNodes) {
          NS_LOG_INFO ("Number of node longitudes does not match number of nodes");
          return 1;
      }
  }
  if (routerLocations.length() > 0) {
      if (routerLocationsVector.size() != numberOfRouters) {
          NS_LOG_INFO ("Number of router locations does not match number of routers");
          return 1;
      }
  }
  if (routerLatitudes.length() > 0) {
      if (routerLatitudesVector.size() != numberOfRouters) {
          NS_LOG_INFO ("Number of router latitudes does not match number of routers");
          return 1;
      }
  }
  if (routerLongitudes.length() > 0) {
      if (routerLongitudesVector.size() != numberOfRouters) {
          NS_LOG_INFO ("Number of router longitudes does not match number of routers");
          return 1;
      }
  }
  if (nodeLatitudes.length() > 0) {
      if (nodeLongitudes.length() == 0) {
          NS_LOG_INFO ("Only specified node Latitudes. Need to also specify node latitudes.");
          return 1;
      }
  }
  if (nodeLongitudes.length() > 0) {
      if (nodeLatitudes.length() == 0) {
          NS_LOG_INFO ("Only specified node Longitudes. Need to also specify node longitudes.");
          return 1;
      }
  }
  if (routerLatitudes.length() > 0) {
      if (routerLongitudes.length() == 0) {
          NS_LOG_INFO ("Only specified router Latitudes. Need to also specify router latitudes.");
          return 1;
      }
  }
  if (routerLongitudes.length() > 0) {
      if (routerLatitudes.length() == 0) {
          NS_LOG_INFO ("Only specified router Longitudes. Need to also specify router longitudes.");
          return 1;
      }
  }
  if (nodeIpAddresses.length() > 0) {
      if (nodeIpAddressesVector.size() != numberOfNodes) {
          NS_LOG_INFO ("Number of nodes Ip Addresses does not match number of nodes");
          return 1;
      }
  }
  if (routerIpAddresses.length() > 0) {
      if (routerIpAddressesVector.size() != numberOfRouters) {
          NS_LOG_INFO ("Number of routers Ip Addresses does not match number of routers");
          return 1;
      }
  }
  
  // If user povided hash powers, then check that they
  // match the number of provided nodes,
  // and also convert the strings into ints.
  std::vector<int> hashPowersIntsVector;
  int totalHashPower = 0;
  if (hashPowers.length() > 0) {
      if (hashPowersVector.size() != numberOfNodes) {
          NS_LOG_INFO ("Did not specify hash power for correct number of nodes");
          return 1;
      }
      int y = 0;
      while (y < numberOfNodes) {
          int power = 0;
          try {
              power = std::stoi(hashPowersVector.at(y));
          } catch (const std::invalid_argument& ia) {
              std::string message = "Did not provide valid hash power for node " + std::to_string(y);
              NS_LOG_INFO (message);
              return 1;
          }
          if (power < 0) {
              std::string message = "Did not provide valid hash power for node " + std::to_string(y);
              NS_LOG_INFO (message);
              return 1;
          }
          hashPowersIntsVector.push_back(power);
          totalHashPower += power;
          y++;
      }
  } else {
      int y = 0;
      while (y < numberOfNodes) {
          hashPowersIntsVector.push_back(10);
          totalHashPower += 10;
          y++;
      }
  }
  std::cout << "Creating network topology" << std::endl;

  if (numberOfRouters > 0) {
      std::cout << "Creating nodes and routers" << std::endl;
  } else {
      std::cout << "Creating nodes" << std::endl;
  }
  // Create the nodes and routers
  // Note that ns-3 terminology refers to both nodes and routers as nodes
  NS_LOG_INFO ("Creating nodes and routers");
  NodeContainer nodes;
  NS_LOG_INFO ("Creating " + std::to_string(numberOfNodes) + " nodes");
  NS_LOG_INFO ("Creating " + std::to_string(numberOfRouters) + " routers");
  nodes.Create (numberOfNodes + numberOfRouters);
  InternetStackHelper internet;
  internet.Install (nodes);
  
  int m = 0;

  // This gives the ip addresses of nodes that are
  // connected to a node in the bitcoin network
  //e.g. nodeConnections.at(0) provides a vector of IP addresses that 
  //are connected to node 0.
  std::vector<std::vector<Ipv4Address>> nodeConnections;

  // Contains the ips of each node (and router)
  // e.g. nodeIps.at(0) is a vector of IP address that node 0 has
  std::vector<std::vector<Ipv4Address>> nodeIps;

  // Contains a map of ip address to associated node number
  std::unordered_map<uint32_t, int> ipNodeNumberMap;

  int n = 0;
  
  while (n < (numberOfNodes+numberOfRouters)) {
      std::vector<Ipv4Address> connections;
      nodeConnections.push_back(connections);
      std::vector<Ipv4Address> ips;
      nodeIps.push_back(ips);
      n++;
  }

  std::cout << "Creating links" << std::endl;
  Ipv4AddressHelper ipv4; 
  ipv4.SetBase ("10.0.0.0", "255.255.255.255", "0.0.0.0");
  int j = 0;
  while (j < numberOfLinks) {

    NS_LOG_INFO ("Attempting to create a link between:");
    std::vector<std::string > nodesVector = stringSplit(linkSubStrings[j], '-');

    if (nodesVector.size() > 2) {
      NS_LOG_INFO ("Links description in incorrect format");
      return 1;
    }

    int values[2]; // the values of the nodes (or routers)
    for (int i = 0; i < 2; i++) {
      std::string nodeNumberString = nodesVector[i].substr(1,-1);
      int value = 0;
      try {
          value = stoi(nodeNumberString);
      } catch (const std::invalid_argument& ia) {
          NS_LOG_INFO ("Incorrect links description");
          return 1;
      }
      
      if (nodesVector[i][0] == 'n') {

        NS_LOG_INFO ("node number " + std::to_string(value));

        if (value >= numberOfNodes || value < 0) {
          NS_LOG_INFO ("Incorrect node number in links description");
          return 1;
        }

        values[i] = value;

      } else if (nodesVector[i][0] == 'r') {

        value = value + numberOfNodes;

        NS_LOG_INFO ("router number " + std::to_string(value - numberOfNodes));

        if (value >= (numberOfNodes+numberOfRouters) || value < numberOfNodes) {
          NS_LOG_INFO ("Incorrect router number in links description");
          return 1;
        }

        values[i] = value;
      } else {
          NS_LOG_INFO ("Links description in incorrect format.");
          return 1;
      }
    }

    if (values[0] == values[1]) {
        if (values[0] < numberOfNodes) {
            NS_LOG_INFO ("Cannot create a link between a node and itself");
            return 1;
        } else {
            NS_LOG_INFO ("Cannot create a link between a router and itself");
            return 1;
        }
    }

    // create the links with chosen datarate and delay
    NodeContainer nodeContainer = NodeContainer (nodes.Get (values[0]), nodes.Get (values[1]));
    PointToPointHelper p2p;
    if (dataRates.length() != 0) {
        NS_LOG_INFO ("with data rate = " + dataRatesVector[j]);
        p2p.SetDeviceAttribute ("DataRate", StringValue (dataRatesVector[j]));
    } else {
        NS_LOG_INFO ("with data rate = " + dataRate);
        p2p.SetDeviceAttribute ("DataRate", StringValue (dataRate));
    }

    if (delays.length() != 0) {
        NS_LOG_INFO ("with delay = " + delaysVector[j]);
        p2p.SetChannelAttribute ("Delay", StringValue (delaysVector[j]));
    } else {
        NS_LOG_INFO ("with delay = " + delay);
        p2p.SetChannelAttribute ("Delay", StringValue (delay));
    }

    // Install a point to point connection between the two nodes (or router/s)
    // in the node container
    NetDeviceContainer netDevice = p2p.Install (nodeContainer);

    // Install an IPv4 address on the nodes/routers
    // ns-3 will chose an ip address and assign it starting from 
    // the base provided previously
    NetDeviceContainer netDevice1 = NetDeviceContainer (netDevice.Get(0));
    NetDeviceContainer netDevice2 = NetDeviceContainer (netDevice.Get(1));
    Ipv4InterfaceContainer interface1 = ipv4.Assign (netDevice1);
    Ipv4InterfaceContainer interface2 = ipv4.Assign (netDevice2);

    if (same) {
        // bcconnections is the same as the links
        (nodeConnections.at(values[0])).push_back(interface2.GetAddress(0));
        (nodeConnections.at(values[1])).push_back(interface1.GetAddress(0));
    }

    (nodeIps.at(values[0])).push_back(interface1.GetAddress(0));
    (nodeIps.at(values[1])).push_back(interface2.GetAddress(0));

    ipNodeNumberMap[interface1.GetAddress(0).Get()] = values[0];
    ipNodeNumberMap[interface2.GetAddress(0).Get()] = values[1];

    j++;
    m+=2;
  }

  j = 0;
  int numberOfConnections = bcConnectionsVector.size();

  // only need to do this, if the bcConnections string is
  // different from the links string
  if (!same) {
    while (j < numberOfConnections) {

        NS_LOG_INFO ("Attempting to create a connection between:");
        std::vector<std::string> nodesVector = stringSplit(bcConnectionsVector[j], '-');

        if (nodesVector.size() > 2) {
        NS_LOG_INFO ("BC connection description in incorrect format");
        return 1;
        }

        int values[2];
        for (int i = 0; i < 2; i++) {
            std::string nodeNumberString = nodesVector[i].substr(1,-1);
            int value = 0;
            try {
                value = stoi(nodeNumberString);
            } catch (const std::invalid_argument& ia) {
                NS_LOG_INFO ("Incorrect BC connections description");
                return 1;
            }

            if (nodesVector[i][0] == 'n') {
                NS_LOG_INFO ("node number " + std::to_string(value));
                if (value >= numberOfNodes || value < 0) {
                NS_LOG_INFO ("Incorrect node number in BC connections description");
                return 1;
                }
                values[i] = value;
            } else {
                NS_LOG_INFO ("BC connections description in incorrect format.");
                return 1;
            }
        }

        if (values[0] == values[1]) {
            NS_LOG_INFO ("Cannot create a connection between a node and itself");
            return 1; 
        }

        // add these to the node connections list
        (nodeConnections.at(values[0])).push_back((nodeIps.at(values[1]).at(0)));
        (nodeConnections.at(values[1])).push_back((nodeIps.at(values[0]).at(0)));

        j++;
    }
  }
  // Added in this log because routing table population
  // can take some time
  NS_LOG_INFO ("About to populate routing tables");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Topology creation successful!");

  NS_LOG_INFO ("Attempting to install BCS app on nodes");

  uint16_t thePort = 8333;

  // Need to convert a number of the 
  // configurable parameters into
  // booleans
  // The rule is that if not 0,
  // then set to true.
  bool simulateTransactions = false;
  if (transactions != 0) {
      simulateTransactions = true;
  }
  bool useCompactBlocks = false;
  if (compactBlocks != 0) {
      useCompactBlocks = true;
  }
  bool testForksBool = false;
  if (testForks != 0) {
      testForksBool = true;
  }
  bool testOrphanBlockBool = false;
  if (testOrphanBlock != 0) {
      testOrphanBlockBool = true;
  }
  bool testCompactBlockTransactionBool = false;
  if (testCompactBlockTransaction != 0) {
      testCompactBlockTransactionBool = true;
  }
   
  // must pass in a positive victim number to
  // BCSBC app or an error will be thrown
  // so set the victim to the number of nodes
  // so no node will be the victim 
  // (as nodes are numbered from 0)
  if (testGetDataTimeoutVictim < 0) {
      testGetDataTimeoutVictim = numberOfNodes;
  }
  
  std::cout << "Installing BCSBC app on nodes" << std::endl;
  int h = 0;
  while (h < (numberOfNodes)) {

    bool testGetDataTimeout=false;
    if (testGetDataTimeoutAttacker == h) {
        // This node is the attacker
        testGetDataTimeout=true;
    } else {
        testGetDataTimeout=false;
    }

    std::string ip = "";
    if (nodeIpAddresses.size()!=0) {
        ip = nodeIpAddressesVector.at(h);
    }

    installBCS(
    nodeConnections.at(h), 
    ip, 
    h, 
    thePort, 
    nodes, 
    ipNodeNumberMap, 
    TCP, 
    blockChainType, 
    numberOfNodes,
    hashPowersIntsVector.at(h),
    totalHashPower,
    simulateTransactions,
    endTime,
    numberTransactionsBlock,
    blockSize,
    transactionSize,
    transactionFee,
    useCompactBlocks,
    testForksBool,
    testOrphanBlockBool,
    testCompactBlockTransactionBool,
    getDataTimeout,
    averageBlockMineInterval,
    averageTransactionCreationInterval,
    blockReward,
    testGetDataTimeoutVictim,
    testGetDataTimeout
    );
    h++;
  }

  NS_LOG_INFO ("Finished installing BCS app on nodes");

  //AsciiTraceHelper ascii;
  //p2p.EnableAsciiAll (ascii.CreateFileStream ("mysim.tr"));
  //p2p.EnablePcapAll ("mysim");
  
  std::cout << "Starting simulation" << std::endl;
  AnimationInterface anim("blockSim.xml");
  Simulator::Run ();
  Simulator::Destroy ();

  std::ofstream myfile5("BCSBCOutput/printblockchain.py", std::ios::app);
  myfile5 << "print_tree(genesis, horizontal=True)" << "\n";
  myfile5.close();

  std::cout << "Simulation complete" << std::endl;

  return 0;
}
