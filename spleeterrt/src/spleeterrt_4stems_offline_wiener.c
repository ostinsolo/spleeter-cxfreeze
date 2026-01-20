#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DR_WAV_IMPLEMENTATION
#include "../third_party/dr_libs/dr_wav.h"
#define DRMP3_IMPLEMENTATION
#include "../third_party/dr_libs/dr_mp3.h"
#define DR_FLAC_IMPLEMENTATION
#include "../third_party/dr_libs/dr_flac.h"

#include "stftFix.h"
#include "spleeter.h"
#include "../third_party/libsamplerate/samplerate.h"

float *decompressedCoefficients = NULL;

static const double compressedCoeffMQ[701] = {
    0.919063234986138511, 0.913619994199411201, 0.897406560667438402, 0.870768836078722797,
    0.834273523109754001, 0.788693711602254766, 0.734989263333015286, 0.674282539592362951,
    0.607830143521649657, 0.536991457245508341, 0.463194839157173466, 0.387902406850539450,
    0.312574364478514499, 0.238633838900749129, 0.167433166845669668, 0.100222526262645231,
    0.038121730693097225, -0.017904091793426027, -0.067064330735278010, -0.108757765594775291,
    -0.142582153044975485, -0.168338357500518510, -0.186029009837402531, -0.195851834439330574,
    -0.198187933840591440, -0.193585458951914369, -0.182739217157973949, -0.166466876941489483,
    -0.145682513177707279, -0.121368299577954988, -0.094545192393702127, -0.066243461643369750,
    -0.037473912797399318, -0.009200603832691301, 0.017684198632122932, 0.042382162574711987,
    0.064207571946511041, 0.082602100079150684, 0.097145355203635028, 0.107561011406507381,
    0.113718474453852247, 0.115630166683615837, 0.113444644504459249, 0.107435882084395071,
    0.097989162055837783, 0.085584105452548076, 0.070775446120293309, 0.054172207614333223,
    0.036415971885660689, 0.018158938330246815, 0.000042459196338912, -0.017323296238410713,
    -0.033377949403731559, -0.047627263300716267, -0.059656793081079629, -0.069142490141500798,
    -0.075858019256578396, -0.079678658979652428, -0.080581767609623323, -0.078643906863599317,
    -0.074034819562284179, -0.067008552930955145, -0.057892102695980191, -0.047072022601671190,
    -0.034979497375161483, -0.022074413174279148, -0.008828977400685476, 0.004288560713652965,
    0.016829555699174666, 0.028379479813981756, 0.038570858162652835, 0.047094241683417769,
    0.053706909605020871, 0.058239076113395197, 0.060597464446319166, 0.060766202425508065,
    0.058805083502616720, 0.054845323789501445, 0.049083025498695095, 0.041770628243703020,
    0.033206689594802247, 0.023724383421121997, 0.013679137601712221, 0.003435850879130631,
    -0.006643868309165797, -0.016214010012738603, -0.024955612937682017, -0.032586990530198853,
    -0.038872417226545809, -0.043629018879643239, -0.046731678346964158, -0.048115839004410917,
    -0.047778163016171563, -0.045775074997070689, -0.042219292770236193, -0.037274512912134725,
    -0.031148477572630149, -0.024084698830208532, -0.016353156105483747, -0.008240309809337577,
    -0.000038789761018515, 0.007962880277736915, 0.015490170167632772, 0.022291712611484882,
    0.028147455724642705, 0.032875542265754551, 0.036337708303315903, 0.038443049556812471,
    0.039150063091460026, 0.038466933347880143, 0.036450092517807633, 0.033201143952433128,
    0.028862291652591764, 0.023610467168487866, 0.017650385903971395, 0.011206796641134264,
    0.004516210167813600, -0.002181595351151269, -0.008651993358287469, -0.014673562407359826,
    -0.020045503214184583, -0.024594176093158650, -0.028178551235573571, -0.030694406321622035,
    -0.032077152831841031, -0.032303222419993387, -0.031389996039150381, -0.029394309376722470,
    -0.026409616804964359, -0.022561940854659814, -0.018004773700230153, -0.012913130046223239,
    -0.007476976122050557, -0.001894276500050309, 0.003636091270827173, 0.008921304789011335,
    0.013781207467236415, 0.018054338893886482, 0.021603186795815136, 0.024318493648450956,
    0.026122487293166251, 0.026970945047679402, 0.026854043263213976, 0.025795987570079431,
    0.023853461640206807, 0.021112972713804853, 0.017687209024348168, 0.013710556397414673,
    0.009333947668859192, 0.004719238361448204, 0.000033314715823999, -0.004557854609777880,
    -0.008895014112733140, -0.012831064739959125, -0.016235971633599879, -0.019000975419769615,
    -0.021041973670496809, -0.022301970824562707, -0.022752529440111541, -0.022394191906072568,
    -0.021255878361951062, -0.019393302279828196, -0.016886478718542881, -0.013836430536684995,
    -0.010361223853106050, -0.006591484944463394, -0.002665565936317155, 0.001275464342459697,
    0.005092825309417521, 0.008654850008311749, 0.011841465274590917, 0.014548176385701671,
    0.016689426206986688, 0.018201223316688792, 0.019042961289876651, 0.019198381208357294,
    0.018675660452129358, 0.017506641810096972, 0.015745246837337051, 0.013465145155917528,
    0.010756776112709417, 0.007723840062309154, 0.004479392878420811, 0.001141688626311485,
    -0.002170078649346380, -0.005339982462341837, -0.008259373919338373, -0.010830557217282604,
    -0.012970007990380254, -0.014611030342508765, -0.015705770153788771, -0.016226526478563909,
    -0.016166328657139784, -0.015538773207899238, -0.014377140707818779, -0.012732837794565421,
    -0.010673232279050818, -0.008278969379415602, -0.005640873626063710, -0.002856553527664500,
    -0.000026834265658038, 0.002747852704083721, 0.005370995258360709, 0.007753319014958258,
    0.009815785813909824, 0.011492173003678219, 0.012731150958433296, 0.013497795530424229,
    0.013774493273929109, 0.013561219484126362, 0.012875191572278549, 0.011749922250546383,
    0.010233717662138146, 0.008387684262046795, 0.006283324310867826, 0.003999812773708582,
    0.001621057822818793, -0.000767347236997687, -0.003081171091507831, -0.005240483245491547,
    -0.007172383140908554, -0.008813427537033921, -0.010111676574189758, -0.011028293954650051,
    -0.011538653669060464, -0.011632924035784708, -0.011316118830412747, -0.010607624279109693,
    -0.009540229002217340, -0.008158700990423423, -0.006517970800651516, -0.004680992876389242,
    -0.002716366825287035, -0.000695807329823454, 0.001308445056270027, 0.003226179724201707,
    0.004991648959431359, 0.006545794666321473, 0.007838194454033614, 0.008828664063258869,
    0.009488466505815606, 0.009801093167340614, 0.009762597931815446, 0.009381481548192093,
    0.008678139395534967, 0.007683900954968532, 0.006439703144240938, 0.004994451750326167,
    0.003403135115410580, 0.001724761684323746, 0.000020197792912962, -0.001650015947868542,
    -0.003227792151864713, -0.004659494079420105, -0.005897735119564774, -0.006902920847777659,
    -0.007644484483944344, -0.008101778413755888, -0.008264597354555087, -0.008133322264400958,
    -0.007718687720230902, -0.007041188742854554, -0.006130155461650219, -0.005022535167821778,
    -0.003761430832691131, -0.002394452762763960, -0.000971945496911797, 0.000454844825025831,
    0.001835596641714852, 0.003122668316617104, 0.004272722114380925, 0.005248162177843576,
    0.006018339594106877, 0.006560486855847911, 0.006860354383749922, 0.006912532886871314,
    0.006720456790559610, 0.006296095343184246, 0.005659348921651181, 0.004837178114662079,
    0.003862502033291890, 0.002772909691220670, 0.001609233980906675, 0.000414041581645497,
    -0.000769906024675906, -0.001901165407831948, -0.002941044990442539, -0.003854910802244932,
    -0.004613320774623974, -0.005192951162290093, -0.005577286818932973, -0.005757056020769449,
    -0.005730399990410974, -0.005502776877353867, -0.005086609356845171, -0.004500693886508907,
    -0.003769397706347888, -0.002921676615038254, -0.001989952181071211, -0.001008891180284735,
    -0.000014132577398601, 0.000958991766382216, 0.001876697269289887, 0.002707904681562794,
    0.003425276863778077, 0.004006100299408528, 0.004432983601002821, 0.004694352366032693,
    0.004784727410211850, 0.004704781367912914, 0.004461176612085595, 0.004066195127045020,
    0.003537178100163921, 0.002895799341758980, 0.002167201988627001, 0.001379032128141500,
    0.000560405874095270, -0.000259152041390146, -0.001050759947470964, -0.001787184184342184,
    -0.002443762818329620, -0.002999217281793143, -0.003436325772772713, -0.003742437746853627,
    -0.003909814939345641, -0.003935790838753386, -0.003822747153675574, -0.003577912336230492,
    -0.003212993416401785, -0.002743658050835511, -0.002188888608264336, -0.001570234143874397,
    -0.000910989133812205, -0.000235329764723561, 0.000432560641279413, 0.001069345839998360,
    0.001653340745377959, 0.002165238082962834, 0.002588733711954676, 0.002911030869493576,
    0.003123208352342977, 0.003220442832479460, 0.003202080909987641, 0.003071561941475896,
    0.002836197950977609, 0.002506821849485185, 0.002097319592184942, 0.001624065644771860,
    0.001105284094542681, 0.000560359841433201, 0.000009125484808694, -0.000528850236297424,
    -0.001034947274672293, -0.001492128764321782, -0.001885503916298735, -0.002202801129643487,
    -0.002434736758218434, -0.002575269035034838, -0.002621730990172647, -0.002574840645573092,
    -0.002438591168737718, -0.002220027862259672, -0.001928922712789991, -0.001577360593112676,
    -0.001179253996234489, -0.000749805296090004, -0.000304936916893765, 0.000139289578942291,
    0.000567244609490058, 0.000964271897894877, 0.001317181565737385, 0.001614678737960774,
    0.001847714105275549, 0.002009746006742852, 0.002096907004167219, 0.002108071492117578,
    0.002044824491385029, 0.001911335280223238, 0.001714142804308648, 0.001461862761483392,
    0.001164828784166054, 0.000834682161765550, 0.000483925998593740, 0.000125460552453497,
    -0.000227883269383389, -0.000563795658925073, -0.000870909262856999, -0.001139175997920627,
    -0.001360187135536317, -0.001527426803455576, -0.001636451679881492, -0.001684992470943874,
    -0.001672975660262524, -0.001602466894019970, -0.001477540114542411, -0.001304079085075351,
    -0.001089520173828018, -0.000842547115162114, -0.000572749884052837, -0.000290260767615715,
    -0.000005381173403155, 0.000271787324682131, 0.000531694720882059, 0.000765665077860444,
    0.000966179147256847, 0.001127108176071712, 0.001243891947014572, 0.001313656276814221,
    0.001335267482640161, 0.001309323638539731, 0.001238084697878249, 0.001125345675261167,
    0.000976258990458714, 0.000797113715033597, 0.000595080778704776, 0.000377934148912837,
    0.000153758569481225, -0.000069345376995143, -0.000283548328671139, -0.000481561352316614,
    -0.000656878246049437, -0.000803983100343503, -0.000918516742461215, -0.000997397336828164,
    -0.001038892182663637, -0.001042639573678904, -0.001009621396079384, -0.000942088876021349,
    -0.000843445486235168, -0.000718092430499951, -0.000571243299133416, -0.000408715393446694,
    -0.000236705827693455, -0.000061560820167541, 0.000110453421068778, 0.000273370118506192,
    0.000421724138601977, 0.000550730322629083, 0.000656432310144007, 0.000735817450677950,
    0.000786894740519778, 0.000808734131933592, 0.000801466989262860, 0.000766248858509478,
    0.000705187025332569, 0.000621236516650982, 0.000518069214751335, 0.000399921568730665,
    0.000271426983019702, 0.000137439322056229, 0.000002854088234340, -0.000127566289796932,
    -0.000249356967950712, -0.000358499459727905, -0.000451549869015331, -0.000525742663600061,
    -0.000579067066332711, -0.000610314210246149, -0.000619094306052033, -0.000605824164738963,
    -0.000571686465475047, -0.000518563123569660, -0.000448945962885840, -0.000365828604967228,
    -0.000272584032349123, -0.000172832651673869, -0.000070305865763902, 0.000031289837955523,
    0.000128403456337462, 0.000217760218406138, 0.000296468531144650, 0.000362109765670523,
    0.000412808249833617, 0.000447279627497663, 0.000464856578836417, 0.000465491738613502,
    0.000449738470059130, 0.000418710921836804, 0.000374025488711351, 0.000317726390511799,
    0.000252198560764628, 0.000180071382714710, 0.000104117018254314, 0.000027147141711448,
    -0.000048088182130861, -0.000118995940709898, -0.000183226726188442, -0.000238749615318076,
    -0.000283913107803554, -0.000317490431754438, -0.000338708143110164, -0.000347257581417331,
    -0.000343289373546324, -0.000327391777412823, -0.000300554208881314, -0.000264117778630104,
    -0.000219715066774411, -0.000169201669906033, -0.000114582260151285, -0.000057933995063949,
    -0.000001330110803372, 0.000053233576939992, 0.000103906741924272, 0.000149044949497856,
    0.000187260510974094, 0.000217462319983746, 0.000238883649294571, 0.000251097355835207,
    0.000254018413730855, 0.000247894152903089, 0.000233283008194432, 0.000211022966716673,
    0.000182191226994878, 0.000148056842795767, 0.000110028310364557, 0.000069598166140896,
    0.000028286691831679, -0.000012413223177511, -0.000051088131137235, -0.000086451240238396,
    -0.000117383287411682, -0.000142965848616867, -0.000162506184310969, -0.000175553056891817,
    -0.000181903303379713, -0.000181599287597338, -0.000174917679492114, -0.000162350303974188,
    -0.000144578058136870, -0.000122439106161612, -0.000096892719738482, -0.000068980234721197,
    -0.000039784640417051, -0.000010390306964679, 0.000018155708707897, 0.000044879452343343,
    0.000068911789142910, 0.000089515073669816, 0.000106104020030281, 0.000118260259226232,
    0.000125740319217347, 0.000128477011389261, 0.000126574445796332, 0.000120297118433995,
    0.000110053709582710, 0.000096376396848920, 0.000079896615190895, 0.000061318285745045,
    0.000041389584008291, 0.000020874325790195, 0.000000524017729422, -0.000018948449136533,
    -0.000036892585232981, -0.000052740818819573, -0.000066025144314650, -0.000076389469747209,
    -0.000083597404911355, -0.000087535408131387, -0.000088211381288728, -0.000085748963835814,
    -0.000080377921496540, -0.000072421149619506, -0.000062278911057702, -0.000050411001395778,
    -0.000037317578835504, -0.000023519411693731, -0.000009538283954733, 0.000004121739654397,
    0.000016991550435446, 0.000028652179052461, 0.000038747470023706, 0.000046993903986995,
    0.000053187343852112, 0.000057206603209622, 0.000059013855762422, 0.000058652018744224,
    0.000056239347370971, 0.000051961567969763, 0.000046061951828715, 0.000038829788015541,
    0.000030587750191730, 0.000021678669346899, 0.000012452221695327, 0.000003252019725938,
    -0.000005596443768274, -0.000013796601731427, -0.000021090036946349, -0.000027263866219030,
    -0.000032156106624670, -0.000035658928433894, -0.000037719781474555, -0.000038340460301533,
    -0.000037574245901964, -0.000035521325378456, -0.000032322744186229, -0.000028153186597604,
    -0.000023212908194301, -0.000017719158939021, -0.000011897436866734, -0.000005972901266780,
    -0.000000162251446552, 0.000005333655793588, 0.000010336218246169, 0.000014694346087313,
    0.000018288433744639, 0.000021032968627464, 0.000022877753957232, 0.000023807775391314,
    0.000023841789731823, 0.000023029757133319, 0.000021449274538926, 0.000019201196577477,
    0.000016404650185715, 0.000013191660473528, 0.000009701607868611, 0.000006075730729441,
    0.000002451874068818, -0.000001040335292299, -0.000004283732788372, -0.000007177155365100,
    -0.000009638185206925, -0.000011605042076524, -0.000013037603419542, -0.000013917565218600,
    -0.000014247788012456, -0.000014050900469913, -0.000013367256554309, -0.000012252360976939,
    -0.000010773890887184, -0.000009008449384796, -0.000007038188493661, -0.000004947435946560,
    -0.000002819451929447, -0.000000733429418017, 0.000001238164361723, 0.000003031826677860,
    0.000004594780305705, 0.000005886220575305, 0.000006878033995645, 0.000007554995005623,
    0.000007914466875845, 0.000007965650064675, 0.000007728435880102, 0.000007231934716903,
    0.000006512756173231, 0.000005613122895949, 0.000004578901096601, 0.000003457628489583,
    0.000002296615219305, 0.000001141185552533, 0.000000033118183302, -0.000000990668545558,
    -0.000001899152803076, -0.000002667946210802, -0.000003279724008567, -0.000003724353373815,
    -0.000003998736284580, -0.000004106393367485, -0.000004056823505075, -0.000003864680371659,
    -0.000003548811395079, -0.000003131206866384, -0.000002635907089885, -0.000002087913711897,
    -0.000001512147886406, -0.000000932492985725, -0.000000370953442845, 0.000000153045653294,
    0.000000623201057861, 0.000001026706448750, 0.000001354458044511, 0.000001601095747237,
    0.000001764891111188, 0.000001847498832724, 0.000001853592772907, 0.000001790410657756,
    0.000001667233505382, 0.000001494826511430, 0.000001284867642241, 0.000001049388641653,
    0.000000800250692844, 0.000000548673760673, 0.000000304834862491, 0.000000077546377014,
    -0.000000125978796206, -0.000000300272674786, -0.000000441669721214, -0.000000548281621807,
    -0.000000619897641839, -0.000000657821438861, -0.000000664657100282, -0.000000644058349094,
    -0.000000600455333568, -0.000000538773213853, -0.000000464155939282, -0.000000381707256690,
    -0.000000296259201847, -0.000000212176215381, -0.000000133200709942, -0.000000062343516559,
    0.0
};

static void decompressResamplerMQ(const double y[701], float *yi) {
    double breaks[701];
    double coefs[2800];
    double s[701];
    double dx[700];
    double dvdf[700];
    for (int k = 0; k < 700; k++) {
        double r = 0.0014285714285714286 * ((double)k + 1.0) - 0.0014285714285714286 * (double)k;
        dx[k] = r;
        dvdf[k] = (y[k + 1] - y[k]) / r;
    }
    s[0] = ((dx[0] + 0.0057142857142857143) * dx[1] * dvdf[0] + dx[0] * dx[0] * dvdf[1]) / 0.0028571428571428571;
    s[700] = ((dx[699] + 0.0057142857142857828) * dx[698] * dvdf[699] + dx[699] * dx[699] * dvdf[698]) / 0.0028571428571428914;
    breaks[0] = dx[1];
    breaks[700] = dx[698];
    for (int k = 0; k < 699; k++) {
        double r = dx[k + 1];
        s[k + 1] = 3.0 * (r * dvdf[k] + dx[k] * dvdf[k + 1]);
        breaks[k + 1] = 2.0 * (r + dx[k]);
    }
    double r = dx[1] / breaks[0];
    breaks[1] -= r * 0.0028571428571428571;
    s[1] -= r * s[0];
    for (int k = 0; k < 698; k++) {
        r = dx[k + 2] / breaks[k + 1];
        breaks[k + 2] -= r * dx[k];
        s[k + 2] -= r * s[k + 1];
    }
    r = 0.0028571428571428914 / breaks[699];
    breaks[700] -= r * dx[698];
    s[700] -= r * s[699];
    s[700] /= breaks[700];
    for (int k = 698; k >= 0; k--)
        s[k + 1] = (s[k + 1] - dx[k] * s[k + 2]) / breaks[k + 1];
    s[0] = (s[0] - 0.0028571428571428571 * s[1]) / breaks[0];
    for (int k = 0; k < 701; k++)
        breaks[k] = 0.0014285714285714286 * (double)k;
    for (int k = 0; k < 700; k++) {
        double invDx = 1.0 / dx[k];
        double dzzdx = (dvdf[k] - s[k]) * invDx;
        double dzdxdx = (s[k + 1] - dvdf[k]) * invDx;
        coefs[k] = (dzdxdx - dzzdx) * invDx;
        coefs[k + 700] = 2.0 * dzzdx - dzdxdx;
        coefs[k + 1400] = s[k];
        coefs[k + 2100] = y[k];
    }
    double d = 1.0 / 22437.0;
    for (int k = 0; k < 22438; k++) {
        int low_i = 0;
        int low_ip1 = 2;
        int high_i = 701;
        double pos = k * d;
        while (high_i > low_ip1) {
            int mid_i = ((low_i + high_i) + 1) >> 1;
            if (pos >= breaks[mid_i - 1]) {
                low_i = mid_i - 1;
                low_ip1 = mid_i + 1;
            } else {
                high_i = mid_i;
            }
        }
        double xloc = pos - breaks[low_i];
        yi[k] = (float)(xloc * (xloc * (xloc * coefs[low_i] + coefs[low_i + 700]) + coefs[low_i + 1400]) + coefs[low_i + 2100]);
    }
}

static int ensure_resampler_coefficients(void) {
    if (decompressedCoefficients) return 0;
    decompressedCoefficients = (float*)malloc(22438 * sizeof(float));
    if (!decompressedCoefficients) {
        printf("ERROR: Unable to allocate resampler coefficients buffer.\n");
        return -1;
    }
    decompressResamplerMQ(compressedCoeffMQ, decompressedCoefficients);
    return 0;
}

static float* maybe_resample(float* data,
                             unsigned int inputSampleRate,
                             double targetSampleRate,
                             unsigned int channels,
                             drwav_uint64* totalPCMFrameCount) {
    if (!data || !totalPCMFrameCount) {
        free(data);
        return NULL;
    }
    if (channels == 0 || targetSampleRate <= 0.0) {
        printf("ERROR: Invalid audio metadata for resampling.\n");
        free(data);
        return NULL;
    }
    if (fabs(targetSampleRate - (double)inputSampleRate) < 0.5) {
        return data;
    }
    if (ensure_resampler_coefficients() != 0) {
        free(data);
        return NULL;
    }
    double ratio = targetSampleRate / (double)inputSampleRate;
    drwav_uint64 desiredFrames = (drwav_uint64)ceil((*totalPCMFrameCount) * ratio);
    if (desiredFrames == 0) desiredFrames = 1;
    if (*totalPCMFrameCount > (drwav_uint64)LONG_MAX || desiredFrames > (drwav_uint64)LONG_MAX) {
        printf("ERROR: Audio too long for built-in resampler path.\n");
        free(data);
        return NULL;
    }
    float* resampled = (float*)malloc((size_t)(desiredFrames * channels * sizeof(float)));
    if (!resampled) {
        free(data);
        return NULL;
    }
    SRC_DATA srcData;
    memset(&srcData, 0, sizeof(srcData));
    srcData.data_in = data;
    srcData.data_out = resampled;
    srcData.input_frames = (long)(*totalPCMFrameCount);
    srcData.output_frames = (long)desiredFrames;
    srcData.src_ratio = ratio;
    srcData.end_of_input = 1;
    int err = src_simple(&srcData, 0, channels);
    free(data);
    if (err) {
        printf("ERROR: Resampler failed: %s\n", src_strerror(err));
        free(resampled);
        return NULL;
    }
    *totalPCMFrameCount = (drwav_uint64)srcData.output_frames_gen;
    printf("   ↪ Resampled %.0f Hz → %.0f Hz (ratio %.5f)\n",
           (double)inputSampleRate, targetSampleRate, ratio);
    return resampled;
}

static void log_audio_summary(unsigned int channels,
                              double sampleRate,
                              drwav_uint64 frameCount) {
    printf("✅ Audio ready:\n");
    printf("   Channels: %u\n", channels);
    printf("   Sample Rate: %.0f Hz\n", sampleRate);
    printf("   Total Samples: %llu\n",
           (unsigned long long)(frameCount * channels));
    printf("   Duration: %.2f seconds\n\n",
           frameCount / sampleRate);
}

static float* finalize_audio(float* data,
                             unsigned int inputSampleRate,
                             double targetSampleRate,
                             unsigned int channels,
                             drwav_uint64* totalPCMFrameCount) {
    float* processed = maybe_resample(data, inputSampleRate, targetSampleRate, channels, totalPCMFrameCount);
    if (!processed) return NULL;
    log_audio_summary(channels, targetSampleRate, *totalPCMFrameCount);
    return processed;
}

// Get coefficient size from spleeter.c
extern size_t getCoeffSize();

// Simple wall clock timer
#ifdef _WIN32
#include <Windows.h>
static double get_wall_time() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq)) return 0;
    if (!QueryPerformanceCounter(&time)) return 0;
    return (double)time.QuadPart / freq.QuadPart;
}
#else
#include <sys/time.h>
static double get_wall_time() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) return 0;
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
#endif

// --- Model loading (float .dat, no decompression) ---------------------------

static void* loadDatFile(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    char fallbackPath[PATH_MAX];
    const char* openedPath = filename;
    if (!fp) {
        snprintf(fallbackPath, sizeof(fallbackPath), "models/%s", filename);
        fp = fopen(fallbackPath, "rb");
        if (fp) openedPath = fallbackPath;
    }
    if (!fp) {
        printf("ERROR: Cannot open model file: %s (also tried models/%s)\n",
               filename, filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("✅ Loaded %s (%ld bytes / %.1f MB)...\n",
           openedPath, fileSize, fileSize / (1024.0 * 1024.0));

    void* coefficients = malloc((size_t)fileSize);
    if (!coefficients) {
        printf("ERROR: Failed to allocate memory for %s\n", filename);
        fclose(fp);
        return NULL;
    }

    size_t bytesRead = fread(coefficients, 1, (size_t)fileSize, fp);
    fclose(fp);

    if (bytesRead != (size_t)fileSize) {
        printf("ERROR: Failed to read complete file: %s\n", filename);
        free(coefficients);
        return NULL;
    }

    return coefficients;
}

static int lib4stem_loadCoefficients(const char *drum,
                                     const char *bass,
                                     const char *acc,
                                     const char *vocal,
                                     void **coeffProvider) {
    coeffProvider[0] = loadDatFile(drum);
    if (!coeffProvider[0]) return -1;

    coeffProvider[1] = loadDatFile(bass);
    if (!coeffProvider[1]) {
        free(coeffProvider[0]);
        return -1;
    }

    coeffProvider[2] = loadDatFile(acc);
    if (!coeffProvider[2]) {
        free(coeffProvider[0]);
        free(coeffProvider[1]);
        return -1;
    }

    coeffProvider[3] = loadDatFile(vocal);
    if (!coeffProvider[3]) {
        free(coeffProvider[0]);
        free(coeffProvider[1]);
        free(coeffProvider[2]);
        return -1;
    }

    return 0;
}

// --- Audio load / write -----------------------------------------------------

static float* loadAudioFile(const char* filename,
                            double targetSampleRate,
                            unsigned int* channels,
                            drwav_uint64* totalPCMFrameCount) {
    printf("Loading audio file...\n");

    // Try WAV first
    drwav wav;
    if (drwav_init_file(&wav, filename, NULL)) {
        *channels = wav.channels;
        *totalPCMFrameCount = wav.totalPCMFrameCount;

        if (wav.sampleRate != (unsigned int)targetSampleRate) {
            printf("⚠️  Sample rate mismatch: %u Hz (expected %.0f Hz)\n",
                   wav.sampleRate, targetSampleRate);
        }

        float* pSampleData = (float*)malloc((size_t)(*totalPCMFrameCount * *channels * sizeof(float)));
        if (!pSampleData) {
            drwav_uninit(&wav);
            return NULL;
        }
        drwav_read_pcm_frames_f32(&wav, *totalPCMFrameCount, pSampleData);
        drwav_uninit(&wav);

        return finalize_audio(pSampleData,
                              wav.sampleRate,
                              targetSampleRate,
                              *channels,
                              totalPCMFrameCount);
    }

    // Try MP3
    drmp3 mp3;
    if (drmp3_init_file(&mp3, filename, NULL)) {
        *channels = mp3.channels;
        drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
        *totalPCMFrameCount = frameCount;

        float* pSampleData = (float*)malloc((size_t)(frameCount * *channels * sizeof(float)));
        if (!pSampleData) {
            drmp3_uninit(&mp3);
            return NULL;
        }
        drmp3_read_pcm_frames_f32(&mp3, frameCount, pSampleData);
        unsigned int mp3SampleRate = mp3.sampleRate;
        drmp3_uninit(&mp3);
        if (mp3SampleRate != (unsigned int)targetSampleRate) {
            printf("⚠️  Sample rate mismatch: %u Hz (expected %.0f Hz)\n",
                   mp3SampleRate, targetSampleRate);
        }
        return finalize_audio(pSampleData,
                              mp3SampleRate,
                              targetSampleRate,
                              *channels,
                              totalPCMFrameCount);
    }

    // Try FLAC
    unsigned int flacChannels;
    unsigned int flacSampleRate;
    drflac_uint64 flacTotalPCMFrameCount;
    float* flacData = drflac_open_file_and_read_pcm_frames_f32(
        filename, &flacChannels, &flacSampleRate, &flacTotalPCMFrameCount, NULL);
    if (flacData) {
        *channels = flacChannels;
        *totalPCMFrameCount = flacTotalPCMFrameCount;
        if (flacSampleRate != (unsigned int)targetSampleRate) {
            printf("⚠️  Sample rate mismatch: %u Hz (expected %.0f Hz)\n",
                   flacSampleRate, targetSampleRate);
        }
        return finalize_audio(flacData,
                              flacSampleRate,
                              targetSampleRate,
                              *channels,
                              totalPCMFrameCount);
    }

    printf("ERROR: Failed to open audio file: %s\n", filename);
    return NULL;
}

static int writeWAVFile(const char* filename,
                        float* data,
                        drwav_uint64 frameCount,
                        unsigned int channels,
                        unsigned int sampleRate) {
    drwav wav;
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = channels;
    format.sampleRate = sampleRate;
    format.bitsPerSample = 32;

    if (!drwav_init_file_write(&wav, filename, &format, NULL)) {
        return 0;
    }

    drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, frameCount, data);
    drwav_uninit(&wav);

    return (framesWritten == frameCount);
}

// --- Channel helpers (copied from original_main.c) --------------------------

static void channel_splitFloat(float *buffer,
                               unsigned int num_frames,
                               float **chan_buffers,
                               unsigned int num_channels,
                               unsigned int preshift) {
    unsigned int i, samples = num_frames * num_channels;
    for (i = 0; i < samples; i++) {
        chan_buffers[i % num_channels][preshift + i / num_channels] = buffer[i];
    }
}

static int ensure_directory(const char* path) {
    if (!path || !*path) return 0;

    char buffer[PATH_MAX];
    snprintf(buffer, sizeof(buffer), "%s", path);

    size_t len = strlen(buffer);
    if (len == 0) return 0;

    for (size_t i = 1; i < len; ++i) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            char save = buffer[i];
            buffer[i] = '\0';
            if (*buffer) {
                if (MKDIR(buffer) != 0 && errno != EEXIST) {
                    return -1;
                }
            }
            buffer[i] = save;
        }
    }
    if (MKDIR(buffer) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

static int parse_int_option(const char* arg, const char* key, int* outVal) {
    size_t len = strlen(key);
    if (strncmp(arg, key, len) != 0) return 0;
    const char* value = arg + len;
    if (!*value) return -1;
    char* endptr = NULL;
    long parsed = strtol(value, &endptr, 10);
    if (value == endptr) return -1;
    *outVal = (int)parsed;
    return 1;
}

static inline void apply_phase_lock(float mixRe,
                                    float mixIm,
                                    float* reArr[],
                                    float* imArr[],
                                    size_t idx) {
    float sumRe = 0.0f;
    float sumIm = 0.0f;
    for (int s = 0; s < 4; ++s) {
        sumRe += reArr[s][idx];
        sumIm += imArr[s][idx];
    }

    const float eps = 1e-12f;
    float denom = sumRe * sumRe + sumIm * sumIm;
    if (denom < eps) {
        return;
    }

    // Complex ratio: c = mix / sum
    float cRe = (mixRe * sumRe + mixIm * sumIm) / denom;
    float cIm = (mixIm * sumRe - mixRe * sumIm) / denom;

    for (int s = 0; s < 4; ++s) {
        float r = reArr[s][idx];
        float i = imArr[s][idx];
        reArr[s][idx] = r * cRe - i * cIm;
        imArr[s][idx] = r * cIm + i * cRe;
    }
}

static drwav_uint64 clamp_frames_to_buffer(drwav_uint64 frames, unsigned int finalSize) {
    drwav_uint64 maxFrames = (finalSize > FFTSIZE) ? (finalSize - FFTSIZE) : 0;
    if (frames > maxFrames) return maxFrames;
    return frames;
}

static int apply_griffin_lim(OfflineSTFT* st,
                             float* targetReL,
                             float* targetImL,
                             float* targetReR,
                             float* targetImR,
                             size_t spectralframeCount,
                             unsigned int finalSize,
                             drwav_uint64 totalFrames,
                             int iterations,
                             float** outL,
                             float** outR,
                             drwav_uint64* framesToWrite) {
    if (iterations <= 0) return 0;
    size_t specSize = spectralframeCount * (size_t)FFTSIZE;
    const float eps = 1e-12f;

    float* targetMagL = (float*)malloc(specSize * sizeof(float));
    float* targetMagR = (float*)malloc(specSize * sizeof(float));
    if (!targetMagL || !targetMagR) {
        free(targetMagL);
        free(targetMagR);
        return -1;
    }
    for (size_t idx = 0; idx < specSize; ++idx) {
        targetMagL[idx] = sqrtf(targetReL[idx] * targetReL[idx] + targetImL[idx] * targetImL[idx]);
        targetMagR[idx] = sqrtf(targetReR[idx] * targetReR[idx] + targetImR[idx] * targetImR[idx]);
    }

    drwav_uint64 framesLimited = clamp_frames_to_buffer(*framesToWrite, finalSize);

    for (int iter = 0; iter < iterations; ++iter) {
        float* splitL = (float*)calloc(finalSize, sizeof(float));
        float* splitR = (float*)calloc(finalSize, sizeof(float));
        if (!splitL || !splitR) {
            free(splitL);
            free(splitR);
            free(targetMagL);
            free(targetMagR);
            return -1;
        }

        memcpy(splitL + FFTSIZE, *outL, (size_t)framesLimited * sizeof(float));
        memcpy(splitR + FFTSIZE, *outR, (size_t)framesLimited * sizeof(float));

        float *tmpReL = NULL, *tmpImL = NULL, *tmpReR = NULL, *tmpImR = NULL;
        size_t frames = stft(st, splitL, splitR, finalSize, &tmpReL, &tmpImL, &tmpReR, &tmpImR);
        free(splitL);
        free(splitR);
        if (!tmpReL || !tmpImL || !tmpReR || !tmpImR || frames != spectralframeCount) {
            free(tmpReL); free(tmpImL); free(tmpReR); free(tmpImR);
            free(targetMagL); free(targetMagR);
            return -1;
        }

        for (size_t idx = 0; idx < specSize; ++idx) {
            float magL = hypotf(tmpReL[idx], tmpImL[idx]);
            float scaleL = (magL > eps) ? (targetMagL[idx] / magL) : 0.0f;
            tmpReL[idx] *= scaleL;
            tmpImL[idx] *= scaleL;

            float magR = hypotf(tmpReR[idx], tmpImR[idx]);
            float scaleR = (magR > eps) ? (targetMagR[idx] / magR) : 0.0f;
            tmpReR[idx] *= scaleR;
            tmpImR[idx] *= scaleR;
        }

        free(*outL);
        free(*outR);
        *outL = NULL;
        *outR = NULL;

        size_t newLen = istft(st, tmpReL, tmpImL, tmpReR, tmpImR, spectralframeCount, outL, outR);
        free(tmpReL); free(tmpImL); free(tmpReR); free(tmpImR);
        if (!*outL || !*outR) {
            free(targetMagL);
            free(targetMagR);
            return -1;
        }

        drwav_uint64 limited = clamp_frames_to_buffer((drwav_uint64)newLen, finalSize);
        if (limited > totalFrames) limited = totalFrames;
        framesLimited = limited;
        *framesToWrite = limited;
    }

    free(targetMagL);
    free(targetMagR);
    return 0;
}

static int apply_fast_griffin_lim(OfflineSTFT* st,
                                  float* targetReL,
                                  float* targetImL,
                                  float* targetReR,
                                  float* targetImR,
                                  size_t spectralframeCount,
                                  unsigned int finalSize,
                                  drwav_uint64 totalFrames,
                                  int iterations,
                                  float momentum,
                                  float** outL,
                                  float** outR,
                                  drwav_uint64* framesToWrite) {
    if (iterations <= 0) return 0;
    if (momentum < 0.0f) momentum = 0.0f;
    if (momentum > 0.99f) momentum = 0.99f;

    size_t specSize = spectralframeCount * (size_t)FFTSIZE;
    const float eps = 1e-12f;

    float* targetMagL = (float*)malloc(specSize * sizeof(float));
    float* targetMagR = (float*)malloc(specSize * sizeof(float));
    float* prevReL = (float*)calloc(specSize, sizeof(float));
    float* prevImL = (float*)calloc(specSize, sizeof(float));
    float* prevReR = (float*)calloc(specSize, sizeof(float));
    float* prevImR = (float*)calloc(specSize, sizeof(float));
    if (!targetMagL || !targetMagR || !prevReL || !prevImL || !prevReR || !prevImR) {
        free(targetMagL);
        free(targetMagR);
        free(prevReL);
        free(prevImL);
        free(prevReR);
        free(prevImR);
        return -1;
    }

    for (size_t idx = 0; idx < specSize; ++idx) {
        targetMagL[idx] = sqrtf(targetReL[idx] * targetReL[idx] + targetImL[idx] * targetImL[idx]);
        targetMagR[idx] = sqrtf(targetReR[idx] * targetReR[idx] + targetImR[idx] * targetImR[idx]);
    }

    drwav_uint64 framesLimited = clamp_frames_to_buffer(*framesToWrite, finalSize);
    int hasPrev = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        float* splitL = (float*)calloc(finalSize, sizeof(float));
        float* splitR = (float*)calloc(finalSize, sizeof(float));
        if (!splitL || !splitR) {
            free(splitL);
            free(splitR);
            free(targetMagL);
            free(targetMagR);
            free(prevReL);
            free(prevImL);
            free(prevReR);
            free(prevImR);
            return -1;
        }

        memcpy(splitL + FFTSIZE, *outL, (size_t)framesLimited * sizeof(float));
        memcpy(splitR + FFTSIZE, *outR, (size_t)framesLimited * sizeof(float));

        float *tmpReL = NULL, *tmpImL = NULL, *tmpReR = NULL, *tmpImR = NULL;
        size_t frames = stft(st, splitL, splitR, finalSize, &tmpReL, &tmpImL, &tmpReR, &tmpImR);
        free(splitL);
        free(splitR);
        if (!tmpReL || !tmpImL || !tmpReR || !tmpImR || frames != spectralframeCount) {
            free(tmpReL); free(tmpImL); free(tmpReR); free(tmpImR);
            free(targetMagL); free(targetMagR);
            free(prevReL); free(prevImL); free(prevReR); free(prevImR);
            return -1;
        }

        for (size_t idx = 0; idx < specSize; ++idx) {
            float magL = hypotf(tmpReL[idx], tmpImL[idx]);
            float curReL = (magL > eps) ? tmpReL[idx] * targetMagL[idx] / magL : 0.0f;
            float curImL = (magL > eps) ? tmpImL[idx] * targetMagL[idx] / magL : 0.0f;

            if (hasPrev) {
                float accReL = curReL + momentum * (curReL - prevReL[idx]);
                float accImL = curImL + momentum * (curImL - prevImL[idx]);
                float accMagL = hypotf(accReL, accImL);
                if (accMagL > eps) {
                    float scale = targetMagL[idx] / accMagL;
                    accReL *= scale;
                    accImL *= scale;
                } else {
                    accReL = 0.0f;
                    accImL = 0.0f;
                }
                tmpReL[idx] = accReL;
                tmpImL[idx] = accImL;
            } else {
                tmpReL[idx] = curReL;
                tmpImL[idx] = curImL;
            }

            prevReL[idx] = curReL;
            prevImL[idx] = curImL;

            float magR = hypotf(tmpReR[idx], tmpImR[idx]);
            float curReR = (magR > eps) ? tmpReR[idx] * targetMagR[idx] / magR : 0.0f;
            float curImR = (magR > eps) ? tmpImR[idx] * targetMagR[idx] / magR : 0.0f;

            if (hasPrev) {
                float accReR = curReR + momentum * (curReR - prevReR[idx]);
                float accImR = curImR + momentum * (curImR - prevImR[idx]);
                float accMagR = hypotf(accReR, accImR);
                if (accMagR > eps) {
                    float scale = targetMagR[idx] / accMagR;
                    accReR *= scale;
                    accImR *= scale;
                } else {
                    accReR = 0.0f;
                    accImR = 0.0f;
                }
                tmpReR[idx] = accReR;
                tmpImR[idx] = accImR;
            } else {
                tmpReR[idx] = curReR;
                tmpImR[idx] = curImR;
            }

            prevReR[idx] = curReR;
            prevImR[idx] = curImR;
        }

        hasPrev = 1;

        free(*outL);
        free(*outR);
        *outL = NULL;
        *outR = NULL;

        size_t newLen = istft(st, tmpReL, tmpImL, tmpReR, tmpImR, spectralframeCount, outL, outR);
        free(tmpReL); free(tmpImL); free(tmpReR); free(tmpImR);
        if (!*outL || !*outR) {
            free(targetMagL);
            free(targetMagR);
            free(prevReL); free(prevImL); free(prevReR); free(prevImR);
            return -1;
        }

        drwav_uint64 limited = clamp_frames_to_buffer((drwav_uint64)newLen, finalSize);
        if (limited > totalFrames) limited = totalFrames;
        framesLimited = limited;
        *framesToWrite = limited;
    }

    free(targetMagL);
    free(targetMagR);
    free(prevReL);
    free(prevImL);
    free(prevReR);
    free(prevImR);
    return 0;
}

// --- 4-stem separation using STFT + 4 Spleeter nets -------------------------

int main(int argc, char *argv[]) {
    printf("=================================================================\n");
    printf("SpleeterRT 4-Stem Offline Processor (STFT-based)\n");
    printf("=================================================================\n");

    if (argc < 2) {
        printf("Usage: %s <input_audio_file> [options]\n", argv[0]);
        printf("Supported formats: WAV, MP3, FLAC\n");
        printf("Options:\n");
        printf("  --out=<dir>            Output directory for stems (default: alongside input)\n");
        printf("  --phase-lock=0|1       Enforce per-bin phase consistency before ISTFT (default: 0)\n");
        printf("  --phase-mode=none|classic|fast  Select Griffin-Lim variant (default: none)\n");
        printf("  --griffin-lim=N        Number of Griffin-Lim iterations (default: 0)\n");
        return -1;
    }

    const char* inputFile = argv[1];
    const char* outputDir = NULL;
    const char* weightLogPath = NULL;
    int useCustomOutputDir = 0;
    int phaseLockEnabled = 0;
    int griffinLimIterations = 0;
    int drumHissSuppression = 0; // 0: off, 1: mask clamp, 2: time-domain gate
    enum { PHASE_MODE_NONE = 0, PHASE_MODE_CLASSIC = 1, PHASE_MODE_FAST = 2 };
    int phaseMode = PHASE_MODE_NONE;
    enum { BASS_MODE_WIENER = 0, BASS_MODE_BASELINE = 1 };
    int bassMode = BASS_MODE_WIENER;
    int vocalBleedGuard = 1;
    int vocalDrumSub = 0;

    for (int argi = 2; argi < argc; ++argi) {
        const char* arg = argv[argi];
        if (!arg) continue;

        if (strncmp(arg, "--out=", 6) == 0) {
            outputDir = arg + 6;
            useCustomOutputDir = 1;
            continue;
        }

        if (strncmp(arg, "--weight-log=", 13) == 0) {
            weightLogPath = arg + 13;
            continue;
        }

        if (strncmp(arg, "--phase-mode=", 13) == 0) {
            const char* val = arg + 13;
            if (strcmp(val, "none") == 0) {
                phaseMode = PHASE_MODE_NONE;
            } else if (strcmp(val, "classic") == 0) {
                phaseMode = PHASE_MODE_CLASSIC;
            } else if (strcmp(val, "fast") == 0) {
                phaseMode = PHASE_MODE_FAST;
            } else {
                printf("Invalid value for --phase-mode (use none|classic|fast)\n");
                return -1;
            }
            continue;
        }

        int tmpInt = 0;
        int parsed = parse_int_option(arg, "--phase-lock=", &tmpInt);
        if (parsed == 1) { phaseLockEnabled = tmpInt ? 1 : 0; continue; }
        if (parsed == -1) { printf("Invalid value for --phase-lock\n"); return -1; }

        parsed = parse_int_option(arg, "--griffin-lim=", &tmpInt);
        if (parsed == 1) { griffinLimIterations = tmpInt < 0 ? 0 : tmpInt; continue; }
        if (parsed == -1) { printf("Invalid value for --griffin-lim\n"); return -1; }

        parsed = parse_int_option(arg, "--drum-hiss=", &tmpInt);
        if (parsed == 1) { drumHissSuppression = tmpInt; continue; }
        if (parsed == -1) { printf("Invalid value for --drum-hiss\n"); return -1; }

        parsed = parse_int_option(arg, "--vocal-guard=", &tmpInt);
        if (parsed == 1) { vocalBleedGuard = tmpInt ? 1 : 0; continue; }
        if (parsed == -1) { printf("Invalid value for --vocal-guard\n"); return -1; }

        parsed = parse_int_option(arg, "--vocal-drum-sub=", &tmpInt);
        if (parsed == 1) { vocalDrumSub = tmpInt ? 1 : 0; continue; }
        if (parsed == -1) { printf("Invalid value for --vocal-drum-sub\n"); return -1; }


        if (strncmp(arg, "--bass-mode=", 12) == 0) {
            const char* val = arg + 12;
            if (strcmp(val, "wiener") == 0) bassMode = BASS_MODE_WIENER;
            else if (strcmp(val, "baseline") == 0) bassMode = BASS_MODE_BASELINE;
            else {
                printf("Invalid value for --bass-mode (use wiener|baseline)\n");
                return -1;
            }
            continue;
        }

        if (arg[0] != '-' && !useCustomOutputDir) {
            outputDir = arg;
            useCustomOutputDir = 1;
            continue;
        }

        printf("Unknown argument: %s\n", arg);
        return -1;
    }

    if (phaseMode != PHASE_MODE_NONE && griffinLimIterations == 0) {
        griffinLimIterations = 4;
    }

    if (outputDir && ensure_directory(outputDir) != 0) {
        printf("ERROR: Failed to create output directory: %s (errno=%d)\n", outputDir, errno);
        return -1;
    }
    if (outputDir) {
        printf("Custom output directory: %s\n", outputDir);
    }

    printf("Input: %s\n", inputFile);
    const char* phaseModeStr = (phaseMode == PHASE_MODE_CLASSIC) ? "classic"
                           : (phaseMode == PHASE_MODE_FAST) ? "fast"
                                                            : "none";
    printf("Mode: wiener | Phase-lock: %s | Phase refinement: %s (%d iterations)\n",
           phaseLockEnabled ? "ON" : "OFF",
           phaseModeStr,
           griffinLimIterations);
    const char* hissModeStr = (drumHissSuppression == 0) ? "off"
                                 : (drumHissSuppression == 1) ? "mask_clamp"
                                                              : "time_gate";
    printf("Drum hiss suppression: %s\n\n", hissModeStr);
    printf("Bass mode: %s\n\n", (bassMode == BASS_MODE_BASELINE) ? "baseline" : "wiener");
    printf("Vocal bleed guard: %s\n", vocalBleedGuard ? "ON" : "OFF");
    printf("Vocal drum subtraction: %s\n", vocalDrumSub ? "ON" : "OFF");
    printf("Vocal HF clamp: OFF (feature removed)\n\n");

    // Load audio file (resampled externally if needed)
    unsigned int channels = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    float* audioData = loadAudioFile(inputFile, 44100.0, &channels, &totalPCMFrameCount);
    if (!audioData) {
        return -1;
    }

    // Prepare padded stereo buffers for STFT (as in original_main.c)
    unsigned int readcount = (unsigned int)ceil((float)totalPCMFrameCount / (float)FFTSIZE);
    unsigned int finalSize = FFTSIZE * readcount + (FFTSIZE << 1);
    float* splittedBuffer[2];
    splittedBuffer[0] = (float*)calloc(finalSize, sizeof(float));
    splittedBuffer[1] = (float*)calloc(finalSize, sizeof(float));
    if (!splittedBuffer[0] || !splittedBuffer[1]) {
        printf("ERROR: Out of memory for split buffers\n");
        free(audioData);
        free(splittedBuffer[0]);
        free(splittedBuffer[1]);
        return -1;
    }

    channel_splitFloat(audioData,
                       (unsigned int)totalPCMFrameCount,
                       splittedBuffer,
                       channels,
                       FFTSIZE);
    if (channels == 1) {
        memcpy(splittedBuffer[1], splittedBuffer[0],
               finalSize * sizeof(float));
    }
    free(audioData);

    // Load 4-stem models
    printf("Loading 4-stem neural network models (.dat)...\n");
    void* coeffProvider[4];
    if (lib4stem_loadCoefficients("drum4stems.dat",
                                  "bass4stems.dat",
                                  "accompaniment4stems.dat",
                                  "vocal4stems.dat",
                                  coeffProvider) != 0) {
        printf("ERROR: Failed to load model files\n");
        printf("Make sure all 4 .dat files are in this directory:\n");
        printf("  - drum4stems.dat\n");
        printf("  - bass4stems.dat\n");
        printf("  - accompaniment4stems.dat\n");
        printf("  - vocal4stems.dat\n");
        free(splittedBuffer[0]);
        free(splittedBuffer[1]);
        return -1;
    }
    printf("✅ All models loaded successfully!\n\n");

    // STFT
    OfflineSTFT st;
    InitSTFT(&st, 1); // single core STFT/ISTFT for simplicity

    float *reL = NULL, *imL = NULL, *reR = NULL, *imR = NULL;
    size_t spectralframeCount = stft(&st,
                                     splittedBuffer[0],
                                     splittedBuffer[1],
                                     finalSize,
                                     &reL, &imL, &reR, &imR);
    free(splittedBuffer[0]);
    free(splittedBuffer[1]);

    if (!reL || !imL || !reR || !imR) {
        printf("ERROR: STFT failed\n");
        FreeSTFT(&st);
        return -1;
    }

    printf("STFT frames: %zu\n", spectralframeCount);

    // Set analysis parameters (must match training)
    const size_t analyseBinLimit = 1536;
    const size_t timeStep = 256;

    // Allocate per-stem complex spectrograms
    size_t specSize = spectralframeCount * FFTSIZE;
    // re*_stem    -> will hold the Wiener-refined spectrograms
    // re*_base    -> will hold the raw (baseline) masked spectrograms
    float* reL_stem[4];
    float* imL_stem[4];
    float* reR_stem[4];
    float* imR_stem[4];
    for (int s = 0; s < 4; ++s) {
        reL_stem[s] = (float*)calloc(specSize, sizeof(float));
        imL_stem[s] = (float*)calloc(specSize, sizeof(float));
        reR_stem[s] = (float*)calloc(specSize, sizeof(float));
        imR_stem[s] = (float*)calloc(specSize, sizeof(float));
        if (!reL_stem[s] || !imL_stem[s] || !reR_stem[s] || !imR_stem[s]) {
            printf("ERROR: Out of memory for stem spectrograms\n");
            for (int k = 0; k <= s; ++k) {
                free(reL_stem[k]);
                free(imL_stem[k]);
                free(reR_stem[k]);
                free(imR_stem[k]);
            }
            free(reL); free(imL); free(reR); free(imR);
            FreeSTFT(&st);
            return -1;
        }
    }

    // Initialize 4 Spleeter networks (one per stem)
    spleeter nn[4];
    float* maskPtr[4];
    for (int s = 0; s < 4; ++s) {
        nn[s] = (spleeter)allocateSpleeterStr();
        initSpleeter(nn[s],
                     (int)analyseBinLimit,
                     (int)timeStep,
                     1,
                     coeffProvider[s]);
        getMaskPtr(nn[s], &maskPtr[s]);
    }

    // Shared magnitude spectrogram buffer for one block
    float* magnitudeSpectrogram =
        (float*)malloc(timeStep * analyseBinLimit * 2 * sizeof(float));
    if (!magnitudeSpectrogram) {
        printf("ERROR: Out of memory for magnitudeSpectrogram\n");
        for (int s = 0; s < 4; ++s) {
            freeSpleeter(nn[s]);
            free(nn[s]);
        }
        for (int s = 0; s < 4; ++s) {
            free(reL_stem[s]);
            free(imL_stem[s]);
            free(reR_stem[s]);
            free(imR_stem[s]);
        }
        free(reL); free(imL); free(reR); free(imR);
        FreeSTFT(&st);
        return -1;
    }

    double* sumMixMag = NULL;
double* sumStemMag[4] = { NULL, NULL, NULL, NULL };
    if (weightLogPath) {
        sumMixMag = (double*)calloc(analyseBinLimit, sizeof(double));
        for (int s = 0; s < 4; ++s) {
            sumStemMag[s] = (double*)calloc(analyseBinLimit, sizeof(double));
        }
        int allocOK = (sumMixMag != NULL);
        for (int s = 0; s < 4 && allocOK; ++s) {
            allocOK = sumStemMag[s] != NULL;
        }
        if (!allocOK) {
            printf("ERROR: Out of memory for weight logging buffers\n");
            free(sumMixMag);
            for (int s = 0; s < 4; ++s) {
                free(sumStemMag[s]);
            }
            free(magnitudeSpectrogram);
            for (int s = 0; s < 4; ++s) {
                freeSpleeter(nn[s]);
                free(nn[s]);
            }
            for (int s = 0; s < 4; ++s) {
                free(reL_stem[s]);
                free(imL_stem[s]);
                free(reR_stem[s]);
                free(imR_stem[s]);
            }
            free(reL); free(imL); free(reR); free(imR);
            FreeSTFT(&st);
            return -1;
        }
    }


    float* bassBaseReL = NULL;
    float* bassBaseImL = NULL;
    float* bassBaseReR = NULL;
    float* bassBaseImR = NULL;
    if (bassMode == BASS_MODE_BASELINE) {
        bassBaseReL = (float*)calloc(specSize, sizeof(float));
        bassBaseImL = (float*)calloc(specSize, sizeof(float));
        bassBaseReR = (float*)calloc(specSize, sizeof(float));
        bassBaseImR = (float*)calloc(specSize, sizeof(float));
        if (!bassBaseReL || !bassBaseImL || !bassBaseReR || !bassBaseImR) {
            printf("ERROR: Out of memory for bass baseline buffers\n");
            free(bassBaseReL); free(bassBaseImL); free(bassBaseReR); free(bassBaseImR);
            free(sumMixMag);
            for (int s = 0; s < 4; ++s) free(sumStemMag[s]);
            free(magnitudeSpectrogram);
            for (int s = 0; s < 4; ++s) { freeSpleeter(nn[s]); free(nn[s]); }
            for (int s = 0; s < 4; ++s) {
                free(reL_stem[s]); free(imL_stem[s]);
                free(reR_stem[s]); free(imR_stem[s]);
            }
            free(reL); free(imL); free(reR); free(imR);
            FreeSTFT(&st);
            return -1;
        }
    }

    printf("Running 4-stem neural networks...\n");
    double startNN = get_wall_time();

    size_t flrCnt = spectralframeCount / timeStep;
    size_t lastStart = flrCnt * timeStep;
    size_t lastEnd = spectralframeCount - lastStart;

    // Process full blocks
    for (size_t j = 0; j < flrCnt; ++j) {
        size_t nnStep = j * timeStep;

        // Build magnitude spectrogram for this block
        for (size_t nnMaskCursor = 0; nnMaskCursor < timeStep; ++nnMaskCursor) {
            size_t frameIndex = nnStep + nnMaskCursor;
            size_t offset = frameIndex * FFTSIZE;
            for (size_t i = 0; i < analyseBinLimit; ++i) {
                size_t idx = offset + i;
                float lR = reL[idx];
                float lI = imL[idx];
                float rR = reR[idx];
                float rI = imR[idx];
                magnitudeSpectrogram[0 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] =
                    hypotf(lR, lI) * (float)FFTSIZE;
                magnitudeSpectrogram[1 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] =
                    hypotf(rR, rI) * (float)FFTSIZE;
            }
        }

        // Run each stem network on the same magnitude spectrogram
        for (int s = 0; s < 4; ++s) {
            processSpleeter(nn[s], magnitudeSpectrogram, maskPtr[s]);
        }

        // Apply Wiener-refined masks to create per-stem complex spectrograms
        for (size_t nnMaskCursor = 0; nnMaskCursor < timeStep; ++nnMaskCursor) {
            size_t frameIndex = nnStep + nnMaskCursor;
            size_t offset = frameIndex * FFTSIZE;

            for (size_t i = 0; i < analyseBinLimit; ++i) {
                size_t idx = offset + i;

                // Compute mixture magnitude for Wiener weights
                float lR = reL[idx];
                float lI = imL[idx];
                float rR = reR[idx];
                float rI = imR[idx];
                float magL = hypotf(lR, lI);
                float magR = hypotf(rR, rI);

                // Per-stem magnitude estimates and adaptive weighting
                float SL[4], SR[4];
                float wPowL[4], wPowR[4];
                float freqHz = (float)i * 44100.0f / (float)FFTSIZE;
                for (int s = 0; s < 4; ++s) {
                    float maskL = maskPtr[s][0 * (analyseBinLimit * timeStep) +
                                             analyseBinLimit * nnMaskCursor + i];
                    float maskR = maskPtr[s][1 * (analyseBinLimit * timeStep) +
                                             analyseBinLimit * nnMaskCursor + i];
                    SL[s] = maskL * magL;
                    SR[s] = maskR * magR;
                    wPowL[s] = SL[s] * SL[s];
                    wPowR[s] = SR[s] * SR[s];
                    if (bassMode == BASS_MODE_BASELINE && s == 1) {
                        bassBaseReL[idx] = reL[idx] * maskL;
                        bassBaseImL[idx] = imL[idx] * maskL;
                        bassBaseReR[idx] = reR[idx] * maskR;
                        bassBaseImR[idx] = imR[idx] * maskR;
                    }
                }

                if (sumMixMag) {
                    double mixMag = 0.5 * (double)(magL + magR);
                    sumMixMag[i] += mixMag;
                for (int s = 0; s < 4; ++s) {
                        double stemMag = 0.5 * (double)(SL[s] + SR[s]);
                        sumStemMag[s][i] += stemMag;
                    }
                }

                if (drumHissSuppression == 1 && freqHz > 12000.0f) {
                    const float hissThresh = 0.20f;
                    const float epsShare = 1e-12f;
                    float sumPreL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                    if (sumPreL > epsShare) {
                        float ratio = wPowL[0] / sumPreL;
                        if (ratio < hissThresh) {
                            wPowL[0] = 0.0f;
                            sumPreL -= wPowL[0];
                        }
                    }
                    float sumPreR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];
                    if (sumPreR > epsShare) {
                        float ratio = wPowR[0] / sumPreR;
                        if (ratio < hissThresh) {
                            wPowR[0] = 0.0f;
                            sumPreR -= wPowR[0];
                }
                    }
                }

                float sumPowL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                float sumPowR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];

                if (vocalBleedGuard && freqHz > 2200.0f && freqHz < 14000.0f) {
                    const float epsShare = 1e-9f;
                    const float guardThresh = 0.45f;
                    const float guardGap = 0.15f;
                    const float guardRange = 0.35f;
                    const float guardMaxAtten = 0.65f;

                    float denomL = sumPowL + epsShare;
                    float drumShareL = wPowL[0] / denomL;
                    float vocalShareL = wPowL[3] / denomL;
                    if (drumShareL > guardThresh && drumShareL > (vocalShareL + guardGap)) {
                        float severity = (drumShareL - guardThresh) / guardRange;
                        if (severity < 0.0f) severity = 0.0f;
                        if (severity > 1.0f) severity = 1.0f;
                        float reduceFactor = guardMaxAtten * severity;
                        float reduce = wPowL[3] * reduceFactor;
                        if (reduce > 0.0f) {
                            wPowL[3] -= reduce;
                            wPowL[2] += reduce;
                        }
                    }

                    float denomR = sumPowR + epsShare;
                    float drumShareR = wPowR[0] / denomR;
                    float vocalShareR = wPowR[3] / denomR;
                    if (drumShareR > guardThresh && drumShareR > (vocalShareR + guardGap)) {
                        float severity = (drumShareR - guardThresh) / guardRange;
                        if (severity < 0.0f) severity = 0.0f;
                        if (severity > 1.0f) severity = 1.0f;
                        float reduceFactor = guardMaxAtten * severity;
                        float reduce = wPowR[3] * reduceFactor;
                        if (reduce > 0.0f) {
                            wPowR[3] -= reduce;
                            wPowR[2] += reduce;
                        }
                    }

                    sumPowL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                    sumPowR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];
                }
                const float epsW = 1e-9f;
                if (sumPowL < epsW) sumPowL = epsW;
                if (sumPowR < epsW) sumPowR = epsW;

                for (int s = 0; s < 4; ++s) {
                    float wL = wPowL[s] / sumPowL;
                    float wR = wPowR[s] / sumPowR;

                    reL_stem[s][idx] = reL[idx] * wL;
                    imL_stem[s][idx] = imL[idx] * wL;
                    reR_stem[s][idx] = reR[idx] * wR;
                    imR_stem[s][idx] = imR[idx] * wR;
                }
            }
        }
    }

    // Process remaining frames (last partial block), if any
    if (lastEnd > 0) {
        size_t nnStep = lastStart;

        // Build magnitude spectrogram (valid frames)
        for (size_t nnMaskCursor = 0; nnMaskCursor < lastEnd; ++nnMaskCursor) {
            size_t frameIndex = nnStep + nnMaskCursor;
            size_t offset = frameIndex * FFTSIZE;
            for (size_t i = 0; i < analyseBinLimit; ++i) {
                size_t idx = offset + i;
                float lR = reL[idx];
                float lI = imL[idx];
                float rR = reR[idx];
                float rI = imR[idx];
                magnitudeSpectrogram[0 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] =
                    hypotf(lR, lI) * (float)FFTSIZE;
                magnitudeSpectrogram[1 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] =
                    hypotf(rR, rI) * (float)FFTSIZE;
            }
        }
        // Zero-pad remaining time steps
        for (size_t nnMaskCursor = lastEnd; nnMaskCursor < timeStep; ++nnMaskCursor) {
            for (size_t i = 0; i < analyseBinLimit; ++i) {
                magnitudeSpectrogram[0 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] = 0.0f;
                magnitudeSpectrogram[1 * (analyseBinLimit * timeStep) +
                                     analyseBinLimit * nnMaskCursor + i] = 0.0f;
            }
        }

        // Run each stem network
        for (int s = 0; s < 4; ++s) {
            processSpleeter(nn[s], magnitudeSpectrogram, maskPtr[s]);
        }

        // Apply Wiener-refined masks for valid frames only
        for (size_t nnMaskCursor = 0; nnMaskCursor < lastEnd; ++nnMaskCursor) {
            size_t frameIndex = nnStep + nnMaskCursor;
            size_t offset = frameIndex * FFTSIZE;

            for (size_t i = 0; i < analyseBinLimit; ++i) {
                size_t idx = offset + i;
                float lR = reL[idx];
                float lI = imL[idx];
                float rR = reR[idx];
                float rI = imR[idx];
                float magL = hypotf(lR, lI);
                float magR = hypotf(rR, rI);

                float SL[4], SR[4];
                float wPowL[4], wPowR[4];
                float freqHz = (float)i * 44100.0f / (float)FFTSIZE;
                for (int s = 0; s < 4; ++s) {
                    float maskL = maskPtr[s][0 * (analyseBinLimit * timeStep) +
                                             analyseBinLimit * nnMaskCursor + i];
                    float maskR = maskPtr[s][1 * (analyseBinLimit * timeStep) +
                                             analyseBinLimit * nnMaskCursor + i];
                    SL[s] = maskL * magL;
                    SR[s] = maskR * magR;
                    wPowL[s] = SL[s] * SL[s];
                    wPowR[s] = SR[s] * SR[s];
                    if (bassMode == BASS_MODE_BASELINE && s == 1) {
                        bassBaseReL[idx] = reL[idx] * maskL;
                        bassBaseImL[idx] = imL[idx] * maskL;
                        bassBaseReR[idx] = reR[idx] * maskR;
                        bassBaseImR[idx] = imR[idx] * maskR;
                    }
                }

                if (sumMixMag) {
                    double mixMag = 0.5 * (double)(magL + magR);
                    sumMixMag[i] += mixMag;
                for (int s = 0; s < 4; ++s) {
                        double stemMag = 0.5 * (double)(SL[s] + SR[s]);
                        sumStemMag[s][i] += stemMag;
                    }
                }

                if (drumHissSuppression && freqHz > 12000.0f) {
                    const float hissThresh = 0.20f;
                    const float epsShare = 1e-12f;
                    float sumPreL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                    if (sumPreL > epsShare) {
                        float ratio = wPowL[0] / sumPreL;
                        if (ratio < hissThresh) {
                            wPowL[0] = 0.0f;
                        }
                    }
                    float sumPreR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];
                    if (sumPreR > epsShare) {
                        float ratio = wPowR[0] / sumPreR;
                        if (ratio < hissThresh) {
                            wPowR[0] = 0.0f;
                }
                    }
                }

                float sumPowL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                float sumPowR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];

                if (vocalBleedGuard && freqHz > 2200.0f && freqHz < 14000.0f) {
                    const float epsShare = 1e-9f;
                    const float guardThresh = 0.45f;
                    const float guardGap = 0.15f;
                    const float guardRange = 0.35f;
                    const float guardMaxAtten = 0.65f;

                    float denomL = sumPowL + epsShare;
                    float drumShareL = wPowL[0] / denomL;
                    float vocalShareL = wPowL[3] / denomL;
                    if (drumShareL > guardThresh && drumShareL > (vocalShareL + guardGap)) {
                        float severity = (drumShareL - guardThresh) / guardRange;
                        if (severity < 0.0f) severity = 0.0f;
                        if (severity > 1.0f) severity = 1.0f;
                        float reduceFactor = guardMaxAtten * severity;
                        float reduce = wPowL[3] * reduceFactor;
                        if (reduce > 0.0f) {
                            wPowL[3] -= reduce;
                            wPowL[2] += reduce;
                        }
                    }

                    float denomR = sumPowR + epsShare;
                    float drumShareR = wPowR[0] / denomR;
                    float vocalShareR = wPowR[3] / denomR;
                    if (drumShareR > guardThresh && drumShareR > (vocalShareR + guardGap)) {
                        float severity = (drumShareR - guardThresh) / guardRange;
                        if (severity < 0.0f) severity = 0.0f;
                        if (severity > 1.0f) severity = 1.0f;
                        float reduceFactor = guardMaxAtten * severity;
                        float reduce = wPowR[3] * reduceFactor;
                        if (reduce > 0.0f) {
                            wPowR[3] -= reduce;
                            wPowR[2] += reduce;
                        }
                    }

                    sumPowL = wPowL[0] + wPowL[1] + wPowL[2] + wPowL[3];
                    sumPowR = wPowR[0] + wPowR[1] + wPowR[2] + wPowR[3];
                }
                const float epsW = 1e-9f;
                if (sumPowL < epsW) sumPowL = epsW;
                if (sumPowR < epsW) sumPowR = epsW;

                for (int s = 0; s < 4; ++s) {
                    float wL = wPowL[s] / sumPowL;
                    float wR = wPowR[s] / sumPowR;
                    reL_stem[s][idx] = reL[idx] * wL;
                    imL_stem[s][idx] = imL[idx] * wL;
                    reR_stem[s][idx] = reR[idx] * wR;
                    imR_stem[s][idx] = imR[idx] * wR;
                }

                if (drumHissSuppression == 2 && freqHz > 12000.0f) {
                    float drumMagL = hypotf(reL_stem[0][idx], imL_stem[0][idx]);
                    float drumMagR = hypotf(reR_stem[0][idx], imR_stem[0][idx]);
                    float mixMagL = magL;
                    float mixMagR = magR;
                    float drumRatioL = (mixMagL > 1e-6f) ? drumMagL / mixMagL : 0.0f;
                    float drumRatioR = (mixMagR > 1e-6f) ? drumMagR / mixMagR : 0.0f;
                    const float drumMaxShare = 0.25f;
                    if (drumRatioL > drumMaxShare) {
                        float scale = drumMaxShare / drumRatioL;
                        reL_stem[0][idx] *= scale;
                        imL_stem[0][idx] *= scale;
                        float residual = mixMagL * (1.0f - scale * drumRatioL);
                        if (residual > 0.0f) {
                            float add = residual * 0.7f;
                            reL_stem[2][idx] += (reL[idx] / mixMagL) * add;
                            imL_stem[2][idx] += (imL[idx] / mixMagL) * add;
                        }
                    }
                    if (drumRatioR > drumMaxShare) {
                        float scale = drumMaxShare / drumRatioR;
                        reR_stem[0][idx] *= scale;
                        imR_stem[0][idx] *= scale;
                        float residual = mixMagR * (1.0f - scale * drumRatioR);
                        if (residual > 0.0f) {
                            float add = residual * 0.7f;
                            reR_stem[2][idx] += (reR[idx] / mixMagR) * add;
                            imR_stem[2][idx] += (imR[idx] / mixMagR) * add;
                        }
                    }
                }
            }
        }
    }

    // For frequencies above analyseBinLimit, put everything into "other" stem (index 2)
    for (size_t frame = 0; frame < spectralframeCount; ++frame) {
        size_t offset = frame * FFTSIZE;
        for (size_t i = analyseBinLimit; i < FFTSIZE; ++i) {
            size_t idx = offset + i;
            reL_stem[2][idx] = reL[idx];
            imL_stem[2][idx] = imL[idx];
            reR_stem[2][idx] = reR[idx];
            imR_stem[2][idx] = imR[idx];
        }
    }

    printf("Neural network inference + mask refinement took %.3f seconds\n",
           get_wall_time() - startNN);

    if (weightLogPath && sumMixMag) {
        FILE* logfp = fopen(weightLogPath, "w");
        if (!logfp) {
            printf("⚠️  Failed to open weight log file %s\n", weightLogPath);
        } else {
            fprintf(logfp, "bin,freq_hz,mixture,drums,bass,other,vocals\n");
            for (size_t i = 0; i < analyseBinLimit; ++i) {
                double freq = (double)i * 44100.0 / (double)FFTSIZE;
                fprintf(logfp, "%zu,%.4f,%.9f,%.9f,%.9f,%.9f,%.9f\n",
                        i,
                        freq,
                        sumMixMag[i],
                        sumStemMag[0][i],
                        sumStemMag[1][i],
                        sumStemMag[2][i],
                        sumStemMag[3][i]);
            }
            fclose(logfp);
            printf("✅ Wrote weight log to %s\n", weightLogPath);
        }
    }

    if (vocalDrumSub) {
        const float freqStart = 2500.0f;
        const float freqHighFocus = 12000.0f;
        const float freqFull = 22050.0f;
        const float minDrumMag = 1e-5f;
        const float drumVsVocalRatioBase = 0.7f;
        const float drumVsVocalRatioHi = 0.4f;
        const float maxTransferBase = 0.9f;
        const float maxTransferHi = 1.0f;
        const float eps = 1e-9f;

        for (size_t frame = 0; frame < spectralframeCount; ++frame) {
            size_t offset = frame * FFTSIZE;
            for (size_t i = 0; i < FFTSIZE; ++i) {
                float freqHz = (float)i * 44100.0f / (float)FFTSIZE;
                if (freqHz < freqStart) continue;
                float freqWeight;
                if (freqHz >= freqFull) freqWeight = 1.0f;
                else if (freqHz >= freqHighFocus) freqWeight = 1.0f;
                else freqWeight = (freqHz - freqStart) / (freqFull - freqStart);
                if (freqWeight <= 0.0f) continue;

                float ratioReq = (freqHz >= freqHighFocus) ? drumVsVocalRatioHi : drumVsVocalRatioBase;
                float maxTransferCur = (freqHz >= freqHighFocus) ? maxTransferHi : maxTransferBase;

                size_t idx = offset + i;
                for (int ch = 0; ch < 2; ++ch) {
                    float drumRe = (ch == 0) ? reL_stem[0][idx] : reR_stem[0][idx];
                    float drumIm = (ch == 0) ? imL_stem[0][idx] : imR_stem[0][idx];
                    float vocalRe = (ch == 0) ? reL_stem[3][idx] : reR_stem[3][idx];
                    float vocalIm = (ch == 0) ? imL_stem[3][idx] : imR_stem[3][idx];

                    float drumMag = hypotf(drumRe, drumIm);
                    float vocalMag = hypotf(vocalRe, vocalIm);
                    if (drumMag < minDrumMag || vocalMag < eps) continue;
                    if (drumMag < ratioReq * vocalMag) continue;

                    float drumPow = drumMag * drumMag + eps;
                    float realDot = vocalRe * drumRe + vocalIm * drumIm;
                    float imagDot = vocalIm * drumRe - vocalRe * drumIm;
                    float coeffRe = realDot / drumPow;
                    float coeffIm = imagDot / drumPow;

                    float scale = freqWeight * maxTransferCur;
                    coeffRe *= scale;
                    coeffIm *= scale;

                    float projRe = coeffRe * drumRe - coeffIm * drumIm;
                    float projIm = coeffRe * drumIm + coeffIm * drumRe;
                    float projMag = hypotf(projRe, projIm);
                    float maxProj = vocalMag * scale;
                    if (projMag > maxProj && projMag > eps) {
                        float ratio = maxProj / projMag;
                        projRe *= ratio;
                        projIm *= ratio;
                    }
                    if (projMag <= eps) continue;

                    if (ch == 0) {
                        reL_stem[3][idx] -= projRe;
                        imL_stem[3][idx] -= projIm;
                        reL_stem[2][idx] += projRe;
                        imL_stem[2][idx] += projIm;
                    } else {
                        reR_stem[3][idx] -= projRe;
                        imR_stem[3][idx] -= projIm;
                        reR_stem[2][idx] += projRe;
                        imR_stem[2][idx] += projIm;
                    }
                }
            }
        }
    }


    if (phaseLockEnabled) {
        for (size_t frame = 0; frame < spectralframeCount; ++frame) {
            size_t offset = frame * FFTSIZE;
            for (size_t i = 0; i < FFTSIZE; ++i) {
                size_t idx = offset + i;
                apply_phase_lock(reL[idx], imL[idx], reL_stem, imL_stem, idx);
                apply_phase_lock(reR[idx], imR[idx], reR_stem, imR_stem, idx);
            }
        }
    }

    if (bassMode == BASS_MODE_BASELINE && bassBaseReL && bassBaseImL) {
        memcpy(reL_stem[1], bassBaseReL, specSize * sizeof(float));
        memcpy(imL_stem[1], bassBaseImL, specSize * sizeof(float));
        memcpy(reR_stem[1], bassBaseReR, specSize * sizeof(float));
        memcpy(imR_stem[1], bassBaseImR, specSize * sizeof(float));
    }

 const char* stemNames[4] = { "drums", "bass", "other", "vocals" };

    for (int s = 0; s < 4; ++s) {
        printf("Reconstructing stem: %s\n", stemNames[s]);

        float* specReL = reL_stem[s];
        float* specImL = imL_stem[s];
        float* specReR = reR_stem[s];
        float* specImR = imR_stem[s];

        float *outL = NULL, *outR = NULL;
        size_t outLen = istft(&st,
                              specReL, specImL,
                              specReR, specImR,
                              spectralframeCount,
                              &outL, &outR);
        if (!outL || !outR) {
            printf("  ❌ ISTFT failed for stem %s\n", stemNames[s]);
            free(outL);
            free(outR);
            continue;
        }

        if (drumHissSuppression == 2 && s == 0) {
            const float cutoffHz = 2500.0f;
            const float gateThreshold = 0.004f;
            const float release = 0.990f;
            float prevEnv = 0.0f;
            float coeff = expf(-2.0f * (float)M_PI * cutoffHz / 44100.0f);
            float prevLowL = 0.0f;
            float prevLowR = 0.0f;
            drwav_uint64 gateSamples = (drwav_uint64)outLen;
            for (drwav_uint64 n = 0; n < gateSamples; ++n) {
                float inL = outL[n];
                float inR = outR[n];
                float lowL = coeff * prevLowL + (1.0f - coeff) * inL;
                float lowR = coeff * prevLowR + (1.0f - coeff) * inR;
                float highL = inL - lowL;
                float highR = inR - lowR;
                prevLowL = lowL;
                prevLowR = lowR;

                float instEnv = fabsf(highL) + fabsf(highR);
                if (instEnv > prevEnv)
                    prevEnv = instEnv;
                else
                    prevEnv = prevEnv * release + instEnv * (1.0f - release);

                float gateGain = (prevEnv <= gateThreshold)
                                     ? (prevEnv / gateThreshold)
                                     : 1.0f;
                float gatedHighL = highL * gateGain;
                float gatedHighR = highR * gateGain;
                outL[n] = lowL + gatedHighL;
                outR[n] = lowR + gatedHighR;
            }
        }

        drwav_uint64 framesToWrite = totalPCMFrameCount;
        if (outLen < framesToWrite) framesToWrite = (drwav_uint64)outLen;

        int skipGL = (s == 1 && bassMode == BASS_MODE_BASELINE);
        if (!skipGL && phaseMode != PHASE_MODE_NONE && griffinLimIterations > 0) {
            int glResult = 0;
            if (phaseMode == PHASE_MODE_CLASSIC) {
                glResult = apply_griffin_lim(&st,
                                             specReL, specImL,
                                             specReR, specImR,
                                             spectralframeCount,
                                             finalSize,
                                             totalPCMFrameCount,
                                             griffinLimIterations,
                                             &outL,
                                             &outR,
                                             &framesToWrite);
            } else if (phaseMode == PHASE_MODE_FAST) {
                glResult = apply_fast_griffin_lim(&st,
                                                  specReL, specImL,
                                                  specReR, specImR,
                                                  spectralframeCount,
                                                  finalSize,
                                                  totalPCMFrameCount,
                                                  griffinLimIterations,
                                                  0.85f,
                                                  &outL,
                                                  &outR,
                                                  &framesToWrite);
            }
            if (glResult != 0) {
                printf("  ⚠️ Phase refinement failed, continuing with existing audio.\n");
            }
        }

        framesToWrite = clamp_frames_to_buffer(framesToWrite, finalSize);
        
        // Ensure we have enough samples to skip pre-roll and still have framesToWrite
        if (outLen < FFTSIZE + framesToWrite) {
            drwav_uint64 available = (outLen > FFTSIZE) ? (outLen - FFTSIZE) : 0;
            if (available < framesToWrite) framesToWrite = available;
        }

        float* interleaved = (float*)calloc((size_t)framesToWrite * 2, sizeof(float));
        if (!interleaved) {
            printf("  ❌ Out of memory interleaving stem %s\n", stemNames[s]);
            free(outL);
            free(outR);
            continue;
        }
        // Skip the FFTSIZE pre-roll padding that was added during STFT
        // This aligns the output with the original input timing
        for (drwav_uint64 i = 0; i < framesToWrite; ++i) {
            interleaved[i * 2]     = outL[FFTSIZE + i];
            interleaved[i * 2 + 1] = outR[FFTSIZE + i];
        }

        char outputPath[2048];
        if (useCustomOutputDir) {
            snprintf(outputPath, sizeof(outputPath),
                     "%s/%s.wav", outputDir, stemNames[s]);
        } else {
            snprintf(outputPath, sizeof(outputPath),
                     "%s_%s.wav", inputFile, stemNames[s]);
        }

        if (writeWAVFile(outputPath, interleaved, framesToWrite, 2, 44100)) {
            printf("  ✅ %s\n", outputPath);
        } else {
            printf("  ❌ Failed to write %s\n", outputPath);
        }

        free(interleaved);
        free(outL);
        free(outR);
    }

    // Cleanup
    if (sumMixMag) {
        free(sumMixMag);
        for (int s = 0; s < 4; ++s) {
            free(sumStemMag[s]);
        }
    }
    free(magnitudeSpectrogram);
    free(bassBaseReL);
    free(bassBaseImL);
    free(bassBaseReR);
    free(bassBaseImR);
    for (int s = 0; s < 4; ++s) {
        freeSpleeter(nn[s]);
        free(nn[s]);
        free(reL_stem[s]);
        free(imL_stem[s]);
        free(reR_stem[s]);
        free(imR_stem[s]);
    }
    free(reL);
    free(imL);
    free(reR);
    free(imR);

    FreeSTFT(&st);

    for (int s = 0; s < 4; ++s) {
        free(coeffProvider[s]);
    }

    printf("\n🎉 4-stem separation complete!\n");
    printf("=================================================================\n");
    return 0;
}