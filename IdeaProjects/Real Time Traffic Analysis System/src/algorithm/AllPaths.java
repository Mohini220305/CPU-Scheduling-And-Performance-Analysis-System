package algorithm;

import graph.*;
import java.util.*;

public class AllPaths {

    public List<List<Node>> getAllPaths(Graph graph, Node source, Node destination) {

        List<List<Node>> allPaths = new ArrayList<>();
        Set<Node> visited = new HashSet<>();
        List<Node> currentPath = new ArrayList<>();

        dfs(graph, source, destination, visited, currentPath, allPaths);

        return allPaths;
    }

    private void dfs(Graph graph, Node current, Node destination,
                     Set<Node> visited, List<Node> path,
                     List<List<Node>> allPaths) {

        visited.add(current);
        path.add(current);

        if (current.equals(destination)) {
            allPaths.add(new ArrayList<>(path));
        } else {
            for (Edge edge : graph.getNeighbors(current)) {
                Node neighbor = edge.getDestination();

                if (!visited.contains(neighbor)) {
                    dfs(graph, neighbor, destination, visited, path, allPaths);
                }
            }
        }

        path.remove(path.size() - 1);
        visited.remove(current);
    }
}
