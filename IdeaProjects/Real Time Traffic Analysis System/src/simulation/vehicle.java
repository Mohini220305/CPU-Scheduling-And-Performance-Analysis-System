package simulation;

import graph.*;
import java.util.*;

public class vehicle {

    private String vehicleId;
    private Node currentLocation;

    public vehicle(String vehicleId, Node start) {
        this.vehicleId = vehicleId;
        this.currentLocation = start;
    }

    public void moveAlongPath(Graph graph, List<Node> path) {

        System.out.println("\nVehicle " + vehicleId + " starting journey...");

        double totalTime = 0;

        for (int i = 0; i < path.size() - 1; i++) {

            Node from = path.get(i);
            Node to = path.get(i + 1);

            for (Edge edge : graph.getNeighbors(from)) {

                if (edge.getDestination().equals(to)) {

                    System.out.println("Moving from " + from + " → " + to);
                    System.out.println("Traffic Level: " + edge.getTrafficLevel());
                    System.out.println("Travel Time: " + edge.getWeight());

                    totalTime += edge.getWeight();
                    currentLocation = to;

                    try {
                        Thread.sleep(1000); // simulate delay
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        System.out.println("\nVehicle reached destination: " + currentLocation);
        System.out.println("Total Travel Time: " + totalTime);
    }
}
