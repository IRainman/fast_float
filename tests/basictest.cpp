#include "fast_float/fast_float.h"
#include <iomanip>

inline void Assert(bool Assertion) {
  if (!Assertion)
    throw std::runtime_error("bug");
}

template <typename T> std::string to_string(T d) {
  std::string s(64, '\0');
  auto written = std::snprintf(&s[0], s.size(), "%.*e",
                               std::numeric_limits<T>::max_digits10 - 1, d);
  s.resize(written);
  return s;
}

template <typename T> std::string to_long_string(T d) {
  std::string s(4096, '\0');
  auto written = std::snprintf(&s[0], s.size(), "%.*e",
                               std::numeric_limits<T>::max_digits10 * 10, d);
  s.resize(written);
  return s;
}

bool basic_test_32bit(std::string vals) {
  std::cout << " parsing "  << vals << std::endl;
  float result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }

  std::cout << std::hexfloat << result_value << std::endl;
  std::cout << std::dec;
  return true;
}

bool basic_test_32bit(std::string vals, float val) {
  std::cout << " parsing "  << vals << std::endl;
  float result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }
  if (std::isnan(val)) {
    if (!std::isnan(result_value)) {
      std::cerr << "not nan" << result_value << std::endl;
      return false;
    }
  } else if (result_value != val) {
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    std::cerr << std::dec;
    uint32_t word;
    memcpy(&word, &result_value, sizeof(word));
    std::cout << "got mantissa = " << (word & ((1<<23)-1)) << std::endl;
    memcpy(&word, &val, sizeof(word));
    std::cout << "wanted mantissa = " << (word & ((1<<23)-1)) << std::endl;
    std::cerr << "string: " << vals << std::endl;
    return false;
  }
  std::cout << std::hexfloat  << result_value << " == " << val << std::endl;
  std::cout << std::dec;
  return true;
}

bool basic_test_32bit(float val) {
  std::string long_vals = to_long_string(val);
  std::string vals = to_string(val);
  return basic_test_32bit(long_vals, val) && basic_test_32bit(vals, val);
}

bool basic_test_64bit(std::string vals, double val) {
  std::cout << " parsing "  << vals << std::endl;
  double result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }
  if (std::isnan(val)) {
    if (!std::isnan(result_value)) {
      std::cerr << "not nan" << result_value << std::endl;
      return false;
    }
  } else if (result_value != val) {
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    std::cerr << std::dec;
    std::cerr << "string: " << vals << std::endl;
    return false;
  }
  std::cout << std::hexfloat << result_value << " == " << val << std::endl;
  std::cout << std::dec;

  return true;
}
bool basic_test_64bit(double val) {
  std::string long_vals = to_long_string(val);
  std::string vals = to_string(val);
  return basic_test_64bit(long_vals, val) && basic_test_64bit(vals, val);
}

int main() {


  std::cout << "======= 64 bits " << std::endl;
  Assert(basic_test_64bit("2.2250738585072013e-308",2.2250738585072013e-308));
  Assert(basic_test_64bit("-92666518056446206563E3", -92666518056446206563E3));
  Assert(basic_test_64bit("-92666518056446206563E3", -92666518056446206563E3));
  Assert(basic_test_64bit("-42823146028335318693e-128",-42823146028335318693e-128)); 
  Assert(basic_test_64bit("90054602635948575728E72",90054602635948575728E72));
  Assert(basic_test_64bit("1.00000000000000188558920870223463870174566020691753515394643550663070558368373221972569761144603605635692374830246134201063722058e-309", 1.00000000000000188558920870223463870174566020691753515394643550663070558368373221972569761144603605635692374830246134201063722058e-309));
  Assert(basic_test_64bit("0e9999999999999999999999999999", 0));
  Assert(basic_test_32bit("1234456789012345678901234567890e9999999999999999999999999999", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-2139879401095466344511101915470454744.9813888656856943E+272", -std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-2402844368454405395.2", -2402844368454405395.2));
  Assert(basic_test_64bit("2402844368454405395.2", 2402844368454405395.2));
  Assert(basic_test_64bit("7.0420557077594588669468784357561207962098443483187940792729600000e+59", 7.0420557077594588669468784357561207962098443483187940792729600000e+59));
  Assert(basic_test_64bit("7.0420557077594588669468784357561207962098443483187940792729600000e+59", 7.0420557077594588669468784357561207962098443483187940792729600000e+59));
  Assert(basic_test_64bit("-1.7339253062092163730578609458683877051596800000000000000000000000e+42", -1.7339253062092163730578609458683877051596800000000000000000000000e+42));
  Assert(basic_test_64bit("-2.0972622234386619214559824785284023792871122537545728000000000000e+52", -2.0972622234386619214559824785284023792871122537545728000000000000e+52));
  Assert(basic_test_64bit("-1.0001803374372191849407179462120053338028379051879898808320000000e+57", -1.0001803374372191849407179462120053338028379051879898808320000000e+57));
  Assert(basic_test_64bit("-1.8607245283054342363818436991534856973992070520151142825984000000e+58", -1.8607245283054342363818436991534856973992070520151142825984000000e+58));
  Assert(basic_test_64bit("-1.9189205311132686907264385602245237137907390376574976000000000000e+52", -1.9189205311132686907264385602245237137907390376574976000000000000e+52));
  Assert(basic_test_64bit("-2.8184483231688951563253238886553506793085187889855201280000000000e+54", -2.8184483231688951563253238886553506793085187889855201280000000000e+54));
  Assert(basic_test_64bit("-1.7664960224650106892054063261344555646357024359107788800000000000e+53", -1.7664960224650106892054063261344555646357024359107788800000000000e+53));
  Assert(basic_test_64bit("-2.1470977154320536489471030463761883783915110400000000000000000000e+45", -2.1470977154320536489471030463761883783915110400000000000000000000e+45));
  Assert(basic_test_64bit("-4.4900312744003159009338275160799498340862630046359789166919680000e+61", -4.4900312744003159009338275160799498340862630046359789166919680000e+61));
  Assert(basic_test_64bit("+1", 1));
  Assert(basic_test_64bit("1.8e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("1.797693134862315700000000000000001e308", 1.7976931348623157e308));
  Assert(basic_test_64bit("1.832312213213213232132132143451234453123412321321312e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("2e30000000000000000", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("2e3000", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("1.9e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit(3e-324));
  Assert(basic_test_64bit(1.00000006e+09f));
  Assert(basic_test_64bit(4.9406564584124653e-324));
  Assert(basic_test_64bit(4.9406564584124654e-324));
  Assert(basic_test_64bit(2.2250738585072009e-308));
  Assert(basic_test_64bit(2.2250738585072014e-308));
  Assert(basic_test_64bit(1.7976931348623157e308));
  Assert(basic_test_64bit(1.7976931348623158e308));
  Assert(basic_test_64bit("4503599627370496.5", 4503599627370496.5));
  Assert(basic_test_64bit("4503599627475352.5", 4503599627475352.5));
  Assert(basic_test_64bit("4503599627475353.5", 4503599627475353.5));
  Assert(basic_test_64bit("2251799813685248.25", 2251799813685248.25));
  Assert(basic_test_64bit("1125899906842624.125", 1125899906842624.125));
  Assert(basic_test_64bit("1125899906842901.875", 1125899906842901.875));
  Assert(basic_test_64bit("2251799813685803.75", 2251799813685803.75));
  Assert(basic_test_64bit("4503599627370497.5", 4503599627370497.5));
  Assert(basic_test_64bit("45035996.273704995", 45035996.273704995));
  Assert(basic_test_64bit("45035996.273704985", 45035996.273704985));
  Assert(basic_test_64bit("0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000044501477170144022721148195934182639518696390927032912960468522194496444440421538910330590478162701758282983178260792422137401728773891892910553144148156412434867599762821265346585071045737627442980259622449029037796981144446145705102663115100318287949527959668236039986479250965780342141637013812613333119898765515451440315261253813266652951306000184917766328660755595837392240989947807556594098101021612198814605258742579179000071675999344145086087205681577915435923018910334964869420614052182892431445797605163650903606514140377217442262561590244668525767372446430075513332450079650686719491377688478005309963967709758965844137894433796621993967316936280457084866613206797017728916080020698679408551343728867675409720757232455434770912461317493580281734466552734375", 0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000044501477170144022721148195934182639518696390927032912960468522194496444440421538910330590478162701758282983178260792422137401728773891892910553144148156412434867599762821265346585071045737627442980259622449029037796981144446145705102663115100318287949527959668236039986479250965780342141637013812613333119898765515451440315261253813266652951306000184917766328660755595837392240989947807556594098101021612198814605258742579179000071675999344145086087205681577915435923018910334964869420614052182892431445797605163650903606514140377217442262561590244668525767372446430075513332450079650686719491377688478005309963967709758965844137894433796621993967316936280457084866613206797017728916080020698679408551343728867675409720757232455434770912461317493580281734466552734375));
  Assert(basic_test_64bit("0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022250738585072008890245868760858598876504231122409594654935248025624400092282356951787758888037591552642309780950434312085877387158357291821993020294379224223559819827501242041788969571311791082261043971979604000454897391938079198936081525613113376149842043271751033627391549782731594143828136275113838604094249464942286316695429105080201815926642134996606517803095075913058719846423906068637102005108723282784678843631944515866135041223479014792369585208321597621066375401613736583044193603714778355306682834535634005074073040135602968046375918583163124224521599262546494300836851861719422417646455137135420132217031370496583210154654068035397417906022589503023501937519773030945763173210852507299305089761582519159720757232455434770912461317493580281734466552734375", 0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022250738585072008890245868760858598876504231122409594654935248025624400092282356951787758888037591552642309780950434312085877387158357291821993020294379224223559819827501242041788969571311791082261043971979604000454897391938079198936081525613113376149842043271751033627391549782731594143828136275113838604094249464942286316695429105080201815926642134996606517803095075913058719846423906068637102005108723282784678843631944515866135041223479014792369585208321597621066375401613736583044193603714778355306682834535634005074073040135602968046375918583163124224521599262546494300836851861719422417646455137135420132217031370496583210154654068035397417906022589503023501937519773030945763173210852507299305089761582519159720757232455434770912461317493580281734466552734375));
  std::cout << std::endl;

  std::cout << "======= 32 bits " << std::endl;
  Assert(basic_test_32bit("1.1754943508e-38",1.1754943508e-38f));
  Assert(basic_test_32bit("30219.0830078125",30219.0830078125f));
  Assert(basic_test_32bit("16252921.5",16252921.5f)); 
  Assert(basic_test_32bit("5322519.25",5322519.25f)); 
  Assert(basic_test_32bit("3900245.875",3900245.875f)); 
  Assert(basic_test_32bit("1510988.3125",1510988.3125f)); 
  Assert(basic_test_32bit("782262.28125",782262.28125f)); 
  Assert(basic_test_32bit("328381.484375",328381.484375f)); 
  Assert(basic_test_32bit("156782.0703125",156782.0703125f)); 
  Assert(basic_test_32bit("85003.24609375",85003.24609375f)); 
  Assert(basic_test_32bit("43827.048828125",43827.048828125f)); 
  Assert(basic_test_32bit("17419.6494140625",17419.6494140625f)); 
  Assert(basic_test_32bit("15498.36376953125",15498.36376953125f)); 
  Assert(basic_test_32bit("6318.580322265625",6318.580322265625f)); 
  Assert(basic_test_32bit("2525.2840576171875",2525.2840576171875f)); 
  Assert(basic_test_32bit("1370.9265747070312",1370.9265747070312f)); 
  Assert(basic_test_32bit("936.3702087402344",936.3702087402344f)); 
  Assert(basic_test_32bit("411.88682556152344",411.88682556152344f)); 
  Assert(basic_test_32bit("206.50310516357422",206.50310516357422f)); 
  Assert(basic_test_32bit("124.16878890991211",124.16878890991211f)); 
  Assert(basic_test_32bit("50.811574935913086",50.811574935913086f)); 
  Assert(basic_test_32bit("17.486443519592285",17.486443519592285f)); 
  Assert(basic_test_32bit("13.91745138168335",13.91745138168335f)); 
  Assert(basic_test_32bit("7.5464513301849365",0x1.e2f90ep+2f)); 
  Assert(basic_test_32bit("2.687217116355896",2.687217116355896f)); 
  Assert(basic_test_32bit("1.1877630352973938",0x1.30113ep+0f)); 
  Assert(basic_test_32bit("0.7622503340244293",0.7622503340244293f)); 
  Assert(basic_test_32bit("0.30531780421733856",0x1.38a53ap-2f)); 
  Assert(basic_test_32bit("0.21791061013936996",0x1.be47eap-3f)); 
  Assert(basic_test_32bit("0.09289376810193062",0x1.7c7e2ep-4f)); 
  Assert(basic_test_32bit("0.03706067614257336",0.03706067614257336f)); 
  Assert(basic_test_32bit("0.028068351559340954",0.028068351559340954f)); 
  Assert(basic_test_32bit("0.012114629615098238",0x1.8cf8e2p-7f)); 
  Assert(basic_test_32bit("0.004221370676532388",0x1.14a6dap-8f)); 
  Assert(basic_test_32bit("0.002153817447833717",0.002153817447833717f)); 
  Assert(basic_test_32bit("0.0015924838953651488",0x1.a175cap-10f)); 
  Assert(basic_test_32bit("0.0008602388261351734",0.0008602388261351734f)); 
  Assert(basic_test_32bit("0.00036393293703440577",0x1.7d9c82p-12f)); 
  Assert(basic_test_32bit("0.00013746770127909258",0.00013746770127909258)); 
  Assert(basic_test_32bit("16407.9462890625", 16407.9462890625f));
  Assert(basic_test_32bit("1.1754947011469036e-38", 0x1.000006p-126f));
  Assert(basic_test_32bit("7.0064923216240854e-46", 0x1p-149f));
  Assert(basic_test_32bit("8388614.5", 8388614.5f));
  Assert(basic_test_32bit("0e9999999999999999999999999999", 0));
  Assert(basic_test_32bit("1234456789012345678901234567890e9999999999999999999999999999", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("4.7019774032891500318749461488889827112746622270883500860350068251e-38",4.7019774032891500318749461488889827112746622270883500860350068251e-38f));
  Assert(basic_test_32bit("3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679", 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679));
  Assert(basic_test_32bit("2.3509887016445750159374730744444913556373311135441750430175034126e-38", 2.3509887016445750159374730744444913556373311135441750430175034126e-38f));
  Assert(basic_test_32bit("+1", 1));
  Assert(basic_test_32bit("2e3000", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("3.5028234666e38", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("7.0060e-46", 0));
  Assert(basic_test_32bit(1.00000006e+09f));
  Assert(basic_test_32bit(1.4012984643e-45f));
  Assert(basic_test_32bit(1.1754942107e-38f));
  Assert(basic_test_32bit(1.1754943508e-45f));
  Assert(basic_test_32bit("3.4028234664e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("3.4028234665e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("3.4028234666e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625", 0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125", 0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000002350988561514728583455765982071533026645717985517980855365926236850006129930346077117064851336181163787841796875", 0.00000000000000000000000000000000000002350988561514728583455765982071533026645717985517980855365926236850006129930346077117064851336181163787841796875));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000001175494210692441075487029444849287348827052428745893333857174530571588870475618904265502351336181163787841796875", 0.00000000000000000000000000000000000001175494210692441075487029444849287348827052428745893333857174530571588870475618904265502351336181163787841796875));
  std::cout << std::endl;

  std::cout << "All ok" << std::endl;
  return EXIT_SUCCESS;
}
