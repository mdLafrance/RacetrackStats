import re
import os
import sys

# Simple function that writes the actual vertex points of an obj to a file

def objToPoints(target, newFileName=None):
    if not os.path.isfile(target):
        raise Exception("Target file {} doesn't exist".format(target))

    if newFileName is None:
        newFileName = target[:-4] + "_points.txt"

    points = []
    extractedPoints = []

    with open(target, 'r') as f:
        for line in f:
            if line.startswith("v "):
                points.append([float(s) for s in line[2:].split(" ")]) # line starts with "v "

            if line.startswith("f "):
                print(line)
                for triplet in line[2:].split(" "): # line starts with f 
                    index = int(triplet.split("/")[0]) - 1 # OBJ is 1-indexed, convert to 0-index
                    extractedPoints.append(points[index])

    with open(newFileName, 'w') as f:
        f.write("Point data for {}. ({} positions)\n".format(target, len(extractedPoints)))

        k = 0
        for point in extractedPoints:
            f.write(str(point)[1:-1]) # str list representation has [...] braces

            if k < 2:
                f.write(", ")
                k += 1

            else:
                f.write(",\n")
                k = 0


if __name__ == "__main__":
    if not len(sys.argv) >= 2:
        raise Exception("Please provide the name of target obj file to extract points from")

    objToPoints(sys.argv[1])

