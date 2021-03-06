import json
import sys

if __name__ == "__main__":
    print 'translating config IDs to chamber IDs'

    #chambersList = sys.argv[1]
    #hvSettingsList = sys.argv[2]

    HVchToSemiChamberMap = {}
    HVchToRollMap = {}
    
    hvSettingsList = None

    if sys.argv[1]:
        hvSettingsList = sys.argv[1]
    else:
        hvSettingsList = 'ecap_one'

    hvPairsFile = ''

    chamberHVmap = {}
    rollHVmap = {}

    with open(hvSettingsList) as hvsl:
        hvPairsFile = hvsl.read().splitlines()

    counter = 0
    lone = []


    for hvpair in hvPairsFile:
        line = [token for token in hvpair.split(' ')]
        chamberString = line[0]
        hvValue = line[1]
        #print chamberString
        #case is barrel chamber
        if not chamberString in HVchToRollMap:
            HVchToRollMap.update({chamberString:[]})
            
        if chamberString.find('W') is not -1:

            ch = chamberString[:4] + chamberString[8:] + chamberString[3:7]
            #print chamberString
            chamberHVmap[ch] = hvValue
            HVchToSemiChamberMap.update({ch:chamberString})
            lone.append(chamberString)
            counter += 1


        else:
            l = len(chamberString)
            if l == 10:
                ch = chamberString
                ch = ch[:8] + 'CH' + ch[8:]
                chamberHVmap[ch] = hvValue
                lone.append(ch)
                counter += 1
                HVchToSemiChamberMap.update({ch:chamberString})
                #print ch, hvValue
            elif l == 13:
                ch = chamberString[:7] + chamberString[10:]
                #print ch
                ch = ch[:8] + 'CH' + ch[8:]
                chamberHVmap[ch] = hvValue

                HVchToSemiChamberMap.update({ch:chamberString})
                lone.append(ch)
                #print ch, hvValue
                ch = chamberString[:4] + chamberString[7:]
                ch = ch[:8] + 'CH' + ch[8:]
                chamberHVmap[ch] = hvValue

                HVchToSemiChamberMap.update({ch:chamberString})
                lone.append(ch)
                #print ch, hvValue
                counter += 2
            else:
                chamberHVmap[chamberString[:8] + 'CH' + chamberString[8:10]] = hvValue
                lone.append(chamberString[:8] + 'CH' + chamberString[8:10])
                HVchToSemiChamberMap.update({chamberString[:8] + 'CH' + chamberString[8:10]:chamberString})
                #print chamberString[:8] + 'CH' + chamberString[8:10]
                chamberHVmap[chamberString[11:19] + 'CH' + chamberString[19:]] = hvValue
                lone.append(chamberString[11:19] + 'CH' + chamberString[19:])
                HVchToSemiChamberMap.update({chamberString[11:19] + 'CH' + chamberString[19:]:chamberString})
                #print chamberString[11:19] + 'CH' + chamberString[19:]
                counter += 2
    #for k in chamberHVmap.keys():
    #    print k, chamberHVmap[k]

    dict_keys = chamberHVmap.keys()

    #print counter
    #print len(dict_keys)
    #print len(lone)

    diff = list(set(lone) - set(dict_keys))
    #print len(diff)

    #print lone

    rollsList = []

    with open ('rpcMap') as rpcmapfile:
        rpc_map = json.loads(rpcmapfile.read())
        for k in rpc_map:
            for kk in rpc_map[k]:
                rollsList.append(kk)

    #print len(rollsList)

    for chamber in dict_keys:
        chamberMatches = [c for c in rollsList if chamber in c]                         
        HVchToRollMap[HVchToSemiChamberMap[chamber]].extend(chamberMatches)
        for cham in chamberMatches: rollHVmap[cham] = chamberHVmap[chamber]
        
    print json.dumps(HVchToRollMap, indent=1)
        
    #for roll in rollHVmap:
        #print roll, rollHVmap[roll]

    #print json.dumps(HVchToSemiChamberMap, indent = 1)
    #print HVchToRollMap

    #print len(rollHVmap)
