package simulation;

import graph.*;
import java.util.*;

public class TrafficSimulator {

    private Random random = new Random();

    public void updateTraffic(Graph graph) {

        System.out.println("\n===== Updating Traffic Conditions =====");

        for (Node node : graph.getAllNodes()) {

            for (Edge edge : graph.getNeighbors(node)) {

                int vehicles = random.nextInt(15);        // 0-14 vehicles
                double speed = 20 + random.nextInt(60);  // 20-80 km/h

                edge.updateTraffic(vehicles, speed);

                System.out.println(node + " → " +
                        edge.getDestination() +
                        " | Vehicles: " + vehicles +
                        " | Avg Speed: " + speed +
                        " | Traffic: " + edge.getTrafficLevel() +
                        " | Weight: " + edge.getWeight());
            }
        }
    }

    private String generateRandomTraffic() {

        int value = random.nextInt(3);

        switch (value) {
            case 0: return "LOW";
            case 1: return "MEDIUM";
            default: return "HIGH";
        }
    }
}
