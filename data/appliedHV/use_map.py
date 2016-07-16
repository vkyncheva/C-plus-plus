import json
import sys

if __name__ == "__main__":
    with open('HVchannelToRollMap.json') as HVCHtoRollMapFile:
        HVCHtoRollMap = json.loads(HVCHtoRollMapFile.read())

    with open(sys.argv[1]) as HVchToHVMapFile:
        HVchToHVMap = HVchToHVMapFile.read().splitlines()

    for i in HVchToHVMap:
        line = i.split(',')
        HVID = line[0]
        HV = line[1]

        HVchRolls = HVCHtoRollMap[HVID]

        for roll in HVchRolls:
            print roll, HV
