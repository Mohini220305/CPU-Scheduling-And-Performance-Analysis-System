package graph;

public class Edge {

    private Node destination;
    private double baseDistance;
    private double currentWeight;
    private String trafficLevel;

    // NEW FIELDS
    private int vehicleCount;
    private double avgSpeed;

    public Edge(Node destination, double distance) {
        this.destination = destination;
        this.baseDistance = distance;
        this.currentWeight = distance;
        this.trafficLevel = "LOW";
        this.vehicleCount = 0;
        this.avgSpeed = 60; // default speed
    }

    public Node getDestination() { return destination; }
    public double getWeight() { return currentWeight; }
    public String getTrafficLevel() { return trafficLevel; }
    public int getVehicleCount() { return vehicleCount; }
    public double getAvgSpeed() { return avgSpeed; }

    //Traffic Detection Based on Vehicles & Speed
    public void updateTraffic(int vehicles, double speed) {

        this.vehicleCount = vehicles;
        this.avgSpeed = speed;

        if (vehicles < 5 && speed > 50) {
            trafficLevel = "LOW";
            currentWeight = baseDistance;
        }
        else if (vehicles < 10 && speed > 30) {
            trafficLevel = "MEDIUM";
            currentWeight = baseDistance * 1.5;
        }
        else {
            trafficLevel = "HIGH";
            currentWeight = baseDistance * 2;
        }
    }
}
