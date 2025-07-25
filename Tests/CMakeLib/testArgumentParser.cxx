/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"

#include "testCommon.h"

namespace {
struct Result : ArgumentParser::ParseResult
{
  struct SubResult : ParseResult
  {
    ArgumentParser::NonEmpty<std::string> SubCommand;
    bool SubOption = false;
    ArgumentParser::NonEmpty<std::string> SubString;
    ArgumentParser::NonEmpty<std::vector<std::string>> SubList;
    std::vector<cm::string_view> ParsedKeywords;
  };

  bool Option1 = false;
  bool Option2 = false;

  std::string String1;
  cm::optional<std::string> String2;
  cm::optional<std::string> String3;
  ArgumentParser::Maybe<std::string> String4;
  ArgumentParser::NonEmpty<std::string> String5;
  ArgumentParser::NonEmpty<std::string> String6;

  ArgumentParser::NonEmpty<std::vector<std::string>> List1;
  ArgumentParser::NonEmpty<std::vector<std::string>> List2;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List3;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List4;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List5;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> List6;

  std::vector<std::vector<std::string>> Multi1;
  std::vector<std::vector<std::string>> Multi2;
  cm::optional<std::vector<std::vector<std::string>>> Multi3;
  cm::optional<std::vector<std::vector<std::string>>> Multi4;

  cm::optional<std::string> Pos0;
  cm::optional<std::string> Pos1;
  cm::optional<std::string> Pos2;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> TrailingPos;

  bool Func0_ = false;
  ArgumentParser::Continue Func0(cm::string_view)
  {
    Func0_ = true;
    return ArgumentParser::Continue::No;
  }

  std::string Func1_;
  ArgumentParser::Continue Func1(cm::string_view arg)
  {
    Func1_ = std::string(arg);
    return ArgumentParser::Continue::No;
  }

  std::map<std::string, std::vector<std::string>> Func2_;
  ArgumentParser::Continue Func2(cm::string_view key, cm::string_view arg)
  {
    Func2_[std::string(key)].emplace_back(arg);
    return key == "FUNC_2b" ? ArgumentParser::Continue::Yes
                            : ArgumentParser::Continue::No;
  }

  std::vector<std::string> Func3_;
  ArgumentParser::Continue Func3(cm::string_view arg)
  {
    Func3_.emplace_back(arg);
    return ArgumentParser::Continue::Yes;
  }

  std::map<std::string, std::vector<std::string>> Func4_;
  ArgumentParser::Continue Func4(cm::string_view key, cm::string_view arg)
  {
    Func4_[std::string(key)].emplace_back(arg);
    return key == "FUNC_4b" ? ArgumentParser::Continue::Yes
                            : ArgumentParser::Continue::No;
  }

  cm::optional<SubResult> Sub;
  ArgumentParser::NonEmpty<std::string> Parent;

  ArgumentParser::Maybe<std::string> UnboundMaybe{ 'u', 'n', 'b', 'o',
                                                   'u', 'n', 'd' };
  ArgumentParser::MaybeEmpty<std::vector<std::string>> UnboundMaybeEmpty{
    1, "unbound"
  };
  ArgumentParser::NonEmpty<std::vector<std::string>> UnboundNonEmpty{
    1, "unbound"
  };
  ArgumentParser::NonEmpty<std::string> UnboundNonEmptyStr{ 'u', 'n', 'b', 'o',
                                                            'u', 'n', 'd' };

  std::vector<cm::string_view> ParsedKeywords;
};

struct Derived : Result
{
};

std::initializer_list<cm::string_view> const args = {
  /* clang-format off */
  "pos0",                    // position index 0
  "OPTION_1",                // option
  "pos2",                    // position index 2, ignored because after keyword
  // "OPTION_2",             // option that is not present
  "STRING_1",                // string arg missing value
  "STRING_2", "foo", "bar",  // string arg + unparsed value, presence captured
  // "STRING_3",             // string arg that is not present
  "STRING_4",                // string arg allowed to be missing value
  "STRING_5", "foo",         // string arg that is not empty
  "STRING_6", "",            // string arg that is empty
  "LIST_1",                  // list arg missing values
  "LIST_2", "foo", "bar",    // list arg with 2 elems
  "LIST_3", "bar",           // list arg ...
  "LIST_3", "foo",           // ... with continuation
  "LIST_4",                  // list arg missing values, presence captured
  // "LIST_5",               // list arg that is not present
  "LIST_6",                  // list arg allowed to be empty
  "MULTI_2",                 // multi list with 0 lists
  "MULTI_3", "foo", "bar",   // multi list with first list with two elems
  "MULTI_3", "bar", "foo",   // multi list with second list with two elems
  // "MULTI_4",              // multi list arg that is not present
  "FUNC_0",                  // callback arg missing value
  "FUNC_1", "foo", "ign1",   // callback with one arg + unparsed value
  "FUNC_2a", "foo", "ign2",  // callback with keyword-dependent arg count
  "FUNC_2b", "bar", "zot",   // callback with keyword-dependent arg count
  "FUNC_3", "foo", "bar",    // callback with list arg ...
  "FUNC_4a", "foo", "ign4",  // callback with keyword-dependent arg count
  "FUNC_4b", "bar", "zot",   // callback with keyword-dependent arg count
  "SUB_CMD",  "foo",         // switch to subparser and set Sub::SUB_CMD to foo
  "SUB_OPTION",              // subparser option
  "SUB_STRING", "sub_value", // subparser string
  "SUB_LIST", "a", "b", "c", // subparser list
  // Return to main parser (simulate another main option if needed)
  "PARENT", "value",
};

struct ResultTrailingPos : public ArgumentParser::ParseResult
{
  bool Option1 = false;
  ArgumentParser::NonEmpty<std::vector<std::string>> List1;

  cm::optional<std::string> Pos0;
  cm::optional<std::string> Pos1;
  ArgumentParser::NonEmpty<std::vector<std::string>> TrailingPos;
};

struct DerivedTrailingPos : ResultTrailingPos
{
};

std::initializer_list<cm::string_view> const args_trailingpos = {
  /* clang-format off */
  "pos0",                    // position index 0
  "pos1",                    // position index 1
  "pos_trailing0",           // trailing positional 0
  "pos_trailing1",           // trailing positional 1
  "OPTION_1",                // option
  "LIST_1", "foo", "bar",    // list arg with 2 elems
  /* clang-format on */
};

bool verifyResult(Result const& result,
                  std::vector<std::string> const& unparsedArguments)
{
  static std::vector<std::string> const foobar = { "foo", "bar" };
  static std::vector<std::string> const barfoo = { "bar", "foo" };
  static std::vector<std::string> const unbound = { "unbound" };
  static std::vector<cm::string_view> const parsedKeywords = {
    /* clang-format off */
    "OPTION_1",
    "STRING_1",
    "STRING_2",
    "STRING_4",
    "STRING_5",
    "STRING_6",
    "LIST_1",
    "LIST_2",
    "LIST_3",
    "LIST_3",
    "LIST_4",
    "LIST_6",
    "MULTI_2",
    "MULTI_3",
    "MULTI_3",
    "FUNC_0",
    "FUNC_1",
    "FUNC_2a",
    "FUNC_2b",
    "FUNC_3",
    "FUNC_4a",
    "FUNC_4b",
    "SUB_CMD",
    "PARENT",
    /* clang-format on */
  };

  static std::vector<cm::string_view> subParsedKeywords = {
    "SUB_CMD", "SUB_OPTION", "SUB_STRING", "SUB_LIST"
  };

  static std::map<std::string, std::vector<std::string>> const func2map = {
    { "FUNC_2a", { "foo" } }, { "FUNC_2b", { "bar", "zot" } }
  };
  static std::map<std::string, std::vector<std::string>> const func4map = {
    { "FUNC_4a", { "foo" } }, { "FUNC_4b", { "bar", "zot" } }
  };
  static std::map<cm::string_view, std::string> const keywordErrors = {
    { "STRING_1"_s, "  missing required value\n" },
    { "STRING_6"_s, "  empty string not allowed\n" },
    { "LIST_1"_s, "  missing required value\n" },
    { "LIST_4"_s, "  missing required value\n" },
    { "FUNC_0"_s, "  missing required value\n" }
  };
  static std::vector<std::string> const unparsed = { "pos2", "bar", "ign1",
                                                     "ign2", "ign4" };

  ASSERT_TRUE(!result);

  ASSERT_TRUE(result.Option1);
  ASSERT_TRUE(!result.Option2);

  ASSERT_TRUE(result.String1.empty());
  ASSERT_TRUE(result.String2);
  ASSERT_TRUE(*result.String2 == "foo");
  ASSERT_TRUE(!result.String3);
  ASSERT_TRUE(result.String4.empty());
  ASSERT_TRUE(result.String5 == "foo");
  ASSERT_TRUE(result.String6.empty());

  ASSERT_TRUE(result.List1.empty());
  ASSERT_TRUE(result.List2 == foobar);
  ASSERT_TRUE(result.List3);
  ASSERT_TRUE(*result.List3 == barfoo);
  ASSERT_TRUE(result.List4);
  ASSERT_TRUE(result.List4->empty());
  ASSERT_TRUE(!result.List5);
  ASSERT_TRUE(result.List6);
  ASSERT_TRUE(result.List6->empty());

  ASSERT_TRUE(result.Multi1.empty());
  ASSERT_TRUE(result.Multi2.size() == 1);
  ASSERT_TRUE(result.Multi2[0].empty());
  ASSERT_TRUE(result.Multi3);
  ASSERT_TRUE((*result.Multi3).size() == 2);
  ASSERT_TRUE((*result.Multi3)[0] == foobar);
  ASSERT_TRUE((*result.Multi3)[1] == barfoo);
  ASSERT_TRUE(!result.Multi4);

  ASSERT_TRUE(result.Pos0 == "pos0");
  ASSERT_TRUE(!result.Pos1);
  ASSERT_TRUE(!result.Pos2);
  ASSERT_TRUE(result.TrailingPos.empty());

  ASSERT_TRUE(result.Func0_ == false);
  ASSERT_TRUE(result.Func1_ == "foo");
  ASSERT_TRUE(result.Func2_ == func2map);
  ASSERT_TRUE(result.Func3_ == foobar);
  ASSERT_TRUE(result.Func4_ == func4map);

  ASSERT_TRUE(unparsedArguments == unparsed);

  ASSERT_TRUE(result.UnboundMaybe == "unbound");
  ASSERT_TRUE(result.UnboundMaybeEmpty == unbound);
  ASSERT_TRUE(result.UnboundNonEmpty == unbound);
  ASSERT_TRUE(result.UnboundNonEmptyStr == "unbound");

  ASSERT_TRUE(result.Sub->SubCommand == "foo");
  ASSERT_TRUE(result.Sub->SubOption);
  ASSERT_TRUE(result.Sub->SubString == "sub_value");
  ASSERT_TRUE(result.Sub->SubList ==
              std::vector<std::string>({ "a", "b", "c" }));
  ASSERT_TRUE(result.Parent == "value");

  ASSERT_TRUE(result.Sub->ParsedKeywords == subParsedKeywords);
  ASSERT_TRUE(result.ParsedKeywords == parsedKeywords);

  ASSERT_TRUE(result.GetKeywordErrors().size() == keywordErrors.size());
  for (auto const& ke : result.GetKeywordErrors()) {
    auto const ki = keywordErrors.find(ke.first);
    ASSERT_TRUE(ki != keywordErrors.end());
    ASSERT_TRUE(ke.second == ki->second);
  }

  return true;
}

bool verifyResult(ResultTrailingPos const& result,
                  std::vector<std::string> const& unparsedArguments)
{
  static std::vector<std::string> const foobar = { "foo", "bar" };
  static std::vector<std::string> const trailing = { "pos_trailing0",
                                                     "pos_trailing1" };

  ASSERT_TRUE(result);
  ASSERT_TRUE(unparsedArguments.empty());

  ASSERT_TRUE(result.Option1);
  ASSERT_TRUE(result.List1 == foobar);

  ASSERT_TRUE(result.Pos0 == "pos0");
  ASSERT_TRUE(result.Pos1 == "pos1");
  ASSERT_TRUE(result.TrailingPos == trailing);

  return true;
}

bool testArgumentParserDynamic()
{
  Result result;
  result.Sub = Result::SubResult();

  std::vector<std::string> unparsedArguments;

  std::function<ArgumentParser::Continue(cm::string_view, cm::string_view)>
    func4 = [&result](cm::string_view key,
                      cm::string_view arg) -> ArgumentParser::Continue {
    return result.Func4(key, arg);
  };

  cmArgumentParser<void> parserDynamic;
  parserDynamic.Bind("OPTION_1"_s, result.Option1);
  ASSERT_TRUE(parserDynamic.HasKeyword("OPTION_1"_s));
  ASSERT_TRUE(!parserDynamic.HasKeyword("NOT_AN_OPTION"_s));

  auto subParser = cmArgumentParser<void>{}
                     .Bind("SUB_CMD"_s, result.Sub->SubCommand)
                     .Bind("SUB_OPTION"_s, result.Sub->SubOption)
                     .Bind("SUB_STRING"_s, result.Sub->SubString)
                     .Bind("SUB_LIST"_s, result.Sub->SubList)
                     .BindParsedKeywords(result.Sub->ParsedKeywords);

  static_cast<ArgumentParser::ParseResult&>(result) =
    cmArgumentParser<void>{}
      .Bind(0, result.Pos0)
      .Bind(1, result.Pos1)
      .Bind(2, result.Pos2)
      .BindTrailingArgs(result.TrailingPos)
      .Bind("OPTION_1"_s, result.Option1)
      .Bind("OPTION_2"_s, result.Option2)
      .Bind("STRING_1"_s, result.String1)
      .Bind("STRING_2"_s, result.String2)
      .Bind("STRING_3"_s, result.String3)
      .Bind("STRING_4"_s, result.String4)
      .Bind("STRING_5"_s, result.String5)
      .Bind("STRING_6"_s, result.String6)
      .Bind("LIST_1"_s, result.List1)
      .Bind("LIST_2"_s, result.List2)
      .Bind("LIST_3"_s, result.List3)
      .Bind("LIST_4"_s, result.List4)
      .Bind("LIST_5"_s, result.List5)
      .Bind("LIST_6"_s, result.List6)
      .Bind("MULTI_1"_s, result.Multi1)
      .Bind("MULTI_2"_s, result.Multi2)
      .Bind("MULTI_3"_s, result.Multi3)
      .Bind("MULTI_4"_s, result.Multi4)
      .Bind("FUNC_0"_s,
            [&result](cm::string_view arg) -> ArgumentParser::Continue {
              return result.Func0(arg);
            })
      .Bind("FUNC_1"_s,
            [&result](cm::string_view arg) -> ArgumentParser::Continue {
              return result.Func1(arg);
            })
      .Bind("FUNC_2a"_s,
            [&result](cm::string_view key, cm::string_view arg)
              -> ArgumentParser::Continue { return result.Func2(key, arg); })
      .Bind("FUNC_2b"_s,
            [&result](cm::string_view key, cm::string_view arg)
              -> ArgumentParser::Continue { return result.Func2(key, arg); })
      .Bind("FUNC_3"_s,
            [&result](cm::string_view arg) -> ArgumentParser::Continue {
              return result.Func3(arg);
            })
      .Bind("FUNC_4a"_s, func4)
      .Bind("FUNC_4b"_s, func4)
      .BindSubParser("SUB_CMD"_s, subParser, result.Sub)
      .Bind("PARENT"_s, result.Parent)
      .BindParsedKeywords(result.ParsedKeywords)
      .Parse(args, &unparsedArguments);

  if (!verifyResult(result, unparsedArguments)) {
    return false;
  }

  unparsedArguments.clear();

  ResultTrailingPos result_trailing;
  static_cast<ArgumentParser::ParseResult&>(result_trailing) =
    cmArgumentParser<void>{}
      .Bind(0, result_trailing.Pos0)
      .Bind(1, result_trailing.Pos1)
      .BindTrailingArgs(result_trailing.TrailingPos)
      .Bind("OPTION_1"_s, result_trailing.Option1)
      .Bind("LIST_1"_s, result_trailing.List1)
      .Parse(args_trailingpos, &unparsedArguments);

  return verifyResult(result_trailing, unparsedArguments);
}

static auto const parserStaticFunc4 =
  [](Result& result, cm::string_view key,
     cm::string_view arg) -> ArgumentParser::Continue {
  return result.Func4(key, arg);
};

#define BIND_ALL(name, resultType)                                            \
  /* Define the sub-parser first */                                           \
  static auto sub##name =                                                     \
    cmArgumentParser<resultType::SubResult>{}                                 \
      .Bind("SUB_CMD"_s, &resultType::SubResult::SubCommand)                  \
      .Bind("SUB_OPTION"_s, &resultType::SubResult::SubOption)                \
      .Bind("SUB_STRING"_s, &resultType::SubResult::SubString)                \
      .Bind("SUB_LIST"_s, &resultType::SubResult::SubList)                    \
      .BindParsedKeywords(&resultType::SubResult::ParsedKeywords);            \
                                                                              \
  /* Define the main parser, which uses the sub-parser */                     \
  static const auto name =                                                    \
    cmArgumentParser<resultType>{}                                            \
      .Bind(0, &resultType::Pos0)                                             \
      .Bind(1, &resultType::Pos1)                                             \
      .Bind(2, &resultType::Pos2)                                             \
      .BindTrailingArgs(&resultType::TrailingPos)                             \
      .Bind("OPTION_1"_s, &resultType::Option1)                               \
      .Bind("OPTION_2"_s, &resultType::Option2)                               \
      .Bind("STRING_1"_s, &resultType::String1)                               \
      .Bind("STRING_2"_s, &resultType::String2)                               \
      .Bind("STRING_3"_s, &resultType::String3)                               \
      .Bind("STRING_4"_s, &resultType::String4)                               \
      .Bind("STRING_5"_s, &resultType::String5)                               \
      .Bind("STRING_6"_s, &resultType::String6)                               \
      .Bind("LIST_1"_s, &resultType::List1)                                   \
      .Bind("LIST_2"_s, &resultType::List2)                                   \
      .Bind("LIST_3"_s, &resultType::List3)                                   \
      .Bind("LIST_4"_s, &resultType::List4)                                   \
      .Bind("LIST_5"_s, &resultType::List5)                                   \
      .Bind("LIST_6"_s, &resultType::List6)                                   \
      .Bind("MULTI_1"_s, &resultType::Multi1)                                 \
      .Bind("MULTI_2"_s, &resultType::Multi2)                                 \
      .Bind("MULTI_3"_s, &resultType::Multi3)                                 \
      .Bind("MULTI_4"_s, &resultType::Multi4)                                 \
      .Bind("FUNC_0"_s, &resultType::Func0)                                   \
      .Bind("FUNC_1"_s, &resultType::Func1)                                   \
      .Bind("FUNC_2a"_s, &resultType::Func2)                                  \
      .Bind("FUNC_2b"_s, &resultType::Func2)                                  \
      .Bind("FUNC_3"_s,                                                       \
            [](resultType& result, cm::string_view arg)                       \
              -> ArgumentParser::Continue { return result.Func3(arg); })      \
      .Bind("FUNC_4a"_s, parserStaticFunc4)                                   \
      .Bind("FUNC_4b"_s, parserStaticFunc4)                                   \
      .BindSubParser("SUB_CMD"_s, sub##name, &resultType::Sub)                \
      .Bind("PARENT"_s, &resultType::Parent)                                  \
      .BindParsedKeywords(&resultType::ParsedKeywords);

BIND_ALL(parserStatic, Result);
BIND_ALL(parserDerivedStatic, Derived);

#define BIND_TRAILING(name, resultType)                                       \
  static auto const name = cmArgumentParser<resultType>{}                     \
                             .Bind(0, &resultType::Pos0)                      \
                             .Bind(1, &resultType::Pos1)                      \
                             .BindTrailingArgs(&resultType::TrailingPos)      \
                             .Bind("OPTION_1"_s, &resultType::Option1)        \
                             .Bind("LIST_1"_s, &resultType::List1)

BIND_TRAILING(parserTrailingStatic, ResultTrailingPos);
BIND_TRAILING(parserTrailingDerivedStatic, DerivedTrailingPos);

bool testArgumentParserStatic()
{
  ASSERT_TRUE(parserStatic.HasKeyword("OPTION_1"_s));
  ASSERT_TRUE(!parserStatic.HasKeyword("NOT_AN_OPTION"_s));

  std::vector<std::string> unparsedArguments;
  Result const result = parserStatic.Parse(args, &unparsedArguments);
  if (!verifyResult(result, unparsedArguments)) {
    return false;
  }

  unparsedArguments.clear();
  ResultTrailingPos const result_trailing =
    parserTrailingStatic.Parse(args_trailingpos, &unparsedArguments);
  return verifyResult(result_trailing, unparsedArguments);
}

bool testArgumentParserDerivedStatic()
{
  ASSERT_TRUE(parserDerivedStatic.HasKeyword("OPTION_1"_s));
  ASSERT_TRUE(!parserDerivedStatic.HasKeyword("NOT_AN_OPTION"_s));

  std::vector<std::string> unparsedArguments;
  Derived const result = parserDerivedStatic.Parse(args, &unparsedArguments);
  if (!verifyResult(result, unparsedArguments)) {
    return false;
  }

  unparsedArguments.clear();
  ResultTrailingPos const result_trailing =
    parserTrailingDerivedStatic.Parse(args_trailingpos, &unparsedArguments);
  return verifyResult(result_trailing, unparsedArguments);
}

bool testArgumentParserStaticBool()
{
  std::vector<std::string> unparsedArguments;
  Result result;
  ASSERT_TRUE(parserStatic.Parse(result, args, &unparsedArguments) == false);
  return verifyResult(result, unparsedArguments);
}

bool testArgumentParserTypes()
{
  ArgumentParser::Maybe<std::string> maybeString;
  maybeString = std::string();
  maybeString = "";

  ArgumentParser::MaybeEmpty<std::vector<std::string>> maybeEmptyVecStr;
  maybeEmptyVecStr = std::vector<std::string>{};

  ArgumentParser::NonEmpty<std::string> nonEmptyString;
  nonEmptyString = std::string("x");
  nonEmptyString = "x";

  ArgumentParser::NonEmpty<std::vector<std::string>> nonEmptyVecStr;
  nonEmptyVecStr = std::vector<std::string>{ "" };
  return true;
}

}

int testArgumentParser(int /*unused*/, char* /*unused*/[])
{
  if (!testArgumentParserDynamic()) {
    std::cout << "While executing testArgumentParserDynamic().\n";
    return -1;
  }

  if (!testArgumentParserStatic()) {
    std::cout << "While executing testArgumentParserStatic().\n";
    return -1;
  }

  if (!testArgumentParserDerivedStatic()) {
    std::cout << "While executing testArgumentParserDerivedStatic().\n";
    return -1;
  }

  if (!testArgumentParserStaticBool()) {
    std::cout << "While executing testArgumentParserStaticBool().\n";
    return -1;
  }

  if (!testArgumentParserTypes()) {
    std::cout << "While executing testArgumentParserTypes().\n";
    return -1;
  }

  return 0;
}
