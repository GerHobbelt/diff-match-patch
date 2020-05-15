/*
 * Diff Match and Patch
 * Copyright 2018 The diff-match-patch Authors.
 * https://github.com/google/diff-match-patch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: fraser@google.com (Neil Fraser)
 * ObjC port: jan@geheimwerk.de (Jan Weiß)
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/*
 * Functions for diff, match and patch.
 * Computes the difference between two texts to create a patch.
 * Applies the patch onto another text, allowing for errors.
 */

/*
 * The data structure representing a diff is an NSMutableArray of Diff objects:
 * {Diff(Operation.DIFF_DELETE, "Hello"),
 *  Diff(Operation.DIFF_INSERT, "Goodbye"),
 *  Diff(Operation.DIFF_EQUAL, " world.")}
 * which means: delete "Hello", add "Goodbye" and keep " world."
 */
typedef NS_ENUM(NSUInteger, Operation) {
    DIFF_DELETE = 1,
    DIFF_INSERT = 2,
    DIFF_EQUAL = 3
};

/*
 * Class representing one diff operation.
 */
@interface Diff : NSObject <NSCopying> {
  Operation operation;  // One of: DIFF_INSERT, DIFF_DELETE or DIFF_EQUAL.
  NSString *text;      // The text associated with this diff operation.
}

@property (nonatomic, assign) Operation operation;
@property (nonatomic, copy) NSString *text;

/**
* Constructor.  Initializes the diff with the provided values.
* @param anOperation One of DIFF_INSERT, DIFF_DELETE or DIFF_EQUAL.
* @param aText The text being applied.
*/
+ (instancetype)diffWithOperation:(Operation)anOperation andText:(NSString *)aText;

/**
* Constructor.  Initializes the diff with the provided values.
* @param anOperation One of DIFF_INSERT, DIFF_DELETE or DIFF_EQUAL.
* @param aText The text being applied.
*/
- (instancetype)initWithOperation:(Operation)anOperation andText:(NSString *)aText;

/**
 * Display a human-readable version of this Diff.
 * @return text version.
 */
- (NSString *)description;

/**
 * Is this Diff equivalent to another Diff?
 * @param obj Another Diff to compare against.
 * @return YES or NO.
 */
- (BOOL)isEqual:(id)obj;

@end

/*
 * Class representing one patch operation.
 */
@interface Patch : NSObject <NSCopying> {
  NSMutableArray *diffs;
  NSUInteger start1;
  NSUInteger start2;
  NSUInteger length1;
  NSUInteger length2;
}

@property (nonatomic, retain) NSArray<Diff *> *diffs;
@property (nonatomic, assign) NSUInteger start1;
@property (nonatomic, assign) NSUInteger start2;
@property (nonatomic, assign) NSUInteger length1;
@property (nonatomic, assign) NSUInteger length2;

/**
* Emulate GNU diff's format.
* Header: @@ -382,8 +481,9 @@
* Indices are printed as 1-based, not 0-based.
* @return The GNU diff NSString.
*/
- (NSString *)description;

@end


/*
 * Class containing the diff, match and patch methods.
 * Also Contains the behaviour settings.
 */
@interface DiffMatchPatch : NSObject {
  // Number of seconds to map a diff before giving up (0 for infinity).
  NSTimeInterval Diff_Timeout;

  // Cost of an empty edit operation in terms of edit characters.
  NSUInteger Diff_EditCost;

  // At what point is no match declared (0.0 = perfection, 1.0 = very loose).
  double Match_Threshold;

  // How far to search for a match (0 = exact location, 1000+ = broad match).
  // A match this many characters away from the expected location will add
  // 1.0 to the score (0.0 is a perfect match).
  NSInteger Match_Distance;

  // When deleting a large block of text (over ~64 characters), how close
  // do the contents have to be to match the expected contents. (0.0 =
  // perfection, 1.0 = very loose).  Note that Match_Threshold controls
  // how closely the end points of a delete need to match.
  float Patch_DeleteThreshold;

  // Chunk size for context length.
  uint16_t Patch_Margin;

  // The number of bits in an int.
  NSUInteger Match_MaxBits;
}

@property (nonatomic, assign) NSTimeInterval Diff_Timeout;
@property (nonatomic, assign) NSUInteger Diff_EditCost;
@property (nonatomic, assign) double Match_Threshold;
@property (nonatomic, assign) NSInteger Match_Distance;
@property (nonatomic, assign) float Patch_DeleteThreshold;
@property (nonatomic, assign) uint16_t Patch_Margin;

/**
* Find the differences between two texts.
* Run a faster, slightly less optimal diff.
* This method allows the 'checklines' of diff_main() to be optional.
* Most of the time checklines is wanted, so default to YES.
* @param text1 Old NSString to be diffed.
* @param text2 New NSString to be diffed.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_mainOfOldString:(NSString *)text1 andNewString:(NSString *)text2;

/**
* Find the differences between two texts.
* @param text1 Old string to be diffed.
* @param text2 New string to be diffed.
* @param checklines Speedup flag.  If NO, then don't run a
*     line-level diff first to identify the changed areas.
*     If YES, then run a faster slightly less optimal diff.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_mainOfOldString:(NSString *)text1 andNewString:(NSString *)text2 checkLines:(BOOL)checklines;

/**
* Determine the common prefix of two strings.
* @param text1 First string.
* @param text2 Second string.
* @return The number of characters common to the start of each string.
*/
- (NSInteger)diff_commonPrefixOfFirstString:(NSString *)text1 andSecondString:(NSString *)text2;

/**
* Determine the common suffix of two strings.
* @param text1 First string.
* @param text2 Second string.
* @return The number of characters common to the end of each string.
*/
- (NSInteger)diff_commonSuffixOfFirstString:(NSString *)text1 andSecondString:(NSString *)text2;

/**
* Reduce the number of edits by eliminating semantically trivial
* equalities.
* @param diffs NSMutableArray of Diff objects.
*/
- (void)diff_cleanupSemantic:(NSMutableArray<Diff *> *)diffs;

/**
* Look for single edits surrounded on both sides by equalities
* which can be shifted sideways to align the edit to a word boundary.
* e.g: The c<ins>at c</ins>ame. -> The <ins>cat </ins>came.
* @param diffs NSMutableArray of Diff objects.
*/
- (void)diff_cleanupSemanticLossless:(NSMutableArray<Diff *> *)diffs;

/**
* Reduce the number of edits by eliminating operationally trivial
* equalities.
* @param diffs NSMutableArray of Diff objects.
*/
- (void)diff_cleanupEfficiency:(NSMutableArray<Diff *> *)diffs;

/**
* Reorder and merge like edit sections.  Merge equalities.
* Any edit section can move as long as it doesn't cross an equality.
* @param diffs NSMutableArray of Diff objects.
*/
- (void)diff_cleanupMerge:(NSMutableArray<Diff *> *)diffs;

/**
* loc is a location in text1, compute and return the equivalent location in
* text2.
* e.g. "The cat" vs "The big cat", 1->1, 5->8
* @param diffs NSMutableArray of Diff objects.
* @param loc Location within text1.
* @return Location within text2.
*/
- (NSUInteger)diff_xIndexIn:(NSArray<Diff *> *)diffs location:(NSUInteger) loc;

/**
* Convert a Diff list into a pretty HTML report.
* @param diffs NSMutableArray of Diff objects.
* @return HTML representation.
*/
- (NSString *)diff_prettyHtml:(NSArray<Diff *> *)diffs;

/**
* Compute and return the source text (all equalities and deletions).
* @param diffs NSMutableArray of Diff objects.
* @return Source text.
*/
- (NSString *)diff_text1:(NSArray<Diff *> *)diffs;

/**
* Compute and return the destination text (all equalities and insertions).
* @param diffs NSMutableArray of Diff objects.
* @return Destination text.
*/
- (NSString *)diff_text2:(NSArray<Diff *> *)diffs;

/**
* Compute the Levenshtein distance; the number of inserted, deleted or
* substituted characters.
* @param diffs NSMutableArray of Diff objects.
* @return Number of changes.
*/
- (NSUInteger)diff_levenshtein:(NSArray<Diff *> *)diffs;

/**
* Crush the diff into an encoded NSString which describes the operations
* required to transform text1 into text2.
* E.g. =3\t-2\t+ing  -> Keep 3 chars, delete 2 chars, insert 'ing'.
* Operations are tab-separated.  Inserted text is escaped using %xx
* notation.
* @param diffs NSMutableArray of Diff objects.
* @return Delta text.
*/
- (NSString *)diff_toDelta:(NSArray<Diff *> *)diffs;

/**
* Given the original text1, and an encoded NSString which describes the
* operations required to transform text1 into text2, compute the full diff.
* @param text1 Source NSString for the diff.
* @param delta Delta text.
* @param error NSError if invalid input.
* @return NSMutableArray of Diff objects or nil if invalid.
*/
- (NSArray<Diff *> * __nullable)diff_fromDeltaWithText:(NSString *)text1 andDelta:(NSString *)delta error:(NSError * __nullable * __null_unspecified)error;

/**
* Locate the best instance of 'pattern' in 'text' near 'loc'.
* Returns NSNotFound if no match found.
* @param text The text to search.
* @param pattern The pattern to search for.
* @param loc The location to search around.
* @return Best match index or NSNotFound.
*/
- (NSInteger)match_mainForText:(NSString *)text pattern:(NSString *)pattern near:(NSInteger)loc;

/**
* Initialise the alphabet for the Bitap algorithm.
* @param pattern The text to encode.
* @return Hash of character locations
*     (NSMutableDictionary: keys:NSString/unichar, values:NSNumber/NSUInteger).
*/
- (NSDictionary<NSString *, NSNumber *> *)match_alphabet:(NSString *)pattern;

/**
* Compute a list of patches to turn text1 into text2.
* A set of diffs will be computed.
* @param text1 Old text.
* @param text2 New text.
* @return NSMutableArray of Patch objects.
*/
- (NSArray<Patch *> *)patch_makeFromOldString:(NSString *)text1 andNewString:(NSString *)text2;

/**
* Compute a list of patches to turn text1 into text2.
* text1 will be derived from the provided diffs.
* @param diffs NSMutableArray of Diff objects for text1 to text2.
* @return NSMutableArray of Patch objects.
*/
- (NSArray<Patch *> *)patch_makeFromDiffs:(NSArray<Diff *> *)diffs;

/**
* Compute a list of patches to turn text1 into text2.
* text2 is ignored, diffs are the delta between text1 and text2.
* @param text1 Old text
* @param text2 New text
* @param diffs NSMutableArray of Diff objects for text1 to text2.
* @return NSMutableArray of Patch objects.
* @deprecated Prefer -patch_makeFromOldString:diffs:.
*/
- (NSArray<Patch *> *)patch_makeFromOldString:(NSString *)text1 newString:(NSString *)text2 diffs:(NSArray<Diff *> *)diffs DEPRECATED_ATTRIBUTE;

/**
* Compute a list of patches to turn text1 into text2.
* text2 is not provided, diffs are the delta between text1 and text2.
* @param text1 Old text.
* @param diffs NSMutableArray of Diff objects for text1 to text2.
* @return NSMutableArray of Patch objects.
*/
- (NSArray<Patch *> *)patch_makeFromOldString:(NSString *)text1 andDiffs:(NSArray<Diff *> *)diffs;

/**
* Given an array of patches, return another array that is identical.
* @param patches NSArray of Patch objects.
* @return NSMutableArray of Patch objects.
*/
- (NSArray<Patch *> *)patch_deepCopy:(NSArray<Patch *> *)patches; // Copy rule applies!

/**
* Merge a set of patches onto the text.  Return a patched text, as well
* as an array of YES/NO values indicating which patches were applied.
* @param sourcePatches NSArray of Patch objects
* @param text Old text.
* @return Two element NSArray, containing the new text and an array of
*      BOOL values.
*/
- (NSArray *)patch_apply:(NSArray<Patch *> *)sourcePatches toString:(NSString *)text;

/**
 * Add some padding on text start and end so that edges can match something.
 * Intended to be called only from within patch_apply.
 * @param patches NSMutableArray of Patch objects.
 * @return The padding NSString added to each side.
 */
- (NSString *)patch_addPadding:(NSArray<Patch *> *)patches;

/**
* Look through the patches and break up any which are longer than the
* maximum limit of the match algorithm.
* Intended to be called only from within patch_apply.
* @param patches NSMutableArray of Patch objects.
*/
- (void)patch_splitMax:(NSMutableArray<Patch *> *)patches;

/**
 * Take a list of patches and return a textual representation.
 * @param patches NSMutableArray of Patch objects.
 * @return Text representation of patches.
 */
- (NSString *)patch_toText:(NSArray<Patch *> *)patches;

/**
* Parse a textual representation of patches and return a NSMutableArray of
* Patch objects.
* @param textline Text representation of patches.
* @param error NSError if invalid input.
* @return NSMutableArray of Patch objects.
*/
- (NSArray<Patch *> * __nullable)patch_fromText:(NSString *)textline error:(NSError * __nullable * __null_unspecified)error;

@end


@interface DiffMatchPatch (PrivateMethods)

/**
* Find the differences between two texts.  Simplifies the problem by
* stripping any common prefix or suffix off the texts before diffing.
* @param text1 Old NSString to be diffed.
* @param text2 New NSString to be diffed.
* @param checklines Speedup flag.  If NO, then don't run a
*     line-level diff first to identify the changed areas.
*     If YES, then run a faster slightly less optimal diff
* @param deadline Time when the diff should be complete by.  Used
*     internally for recursive calls.  Users should set DiffTimeout
*     instead.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_mainOfOldString:(NSString *)text1 andNewString:(NSString *)text2 checkLines:(BOOL)checklines deadline:(NSTimeInterval)deadline;

/**
* Find the differences between two texts.  Assumes that the texts do not
* have any common prefix or suffix.
* @param text1 Old NSString to be diffed.
* @param text2 New NSString to be diffed.
* @param checklines Speedup flag.  If NO, then don't run a
*     line-level diff first to identify the changed areas.
*     If YES, then run a faster slightly less optimal diff.
* @param deadline Time the diff should be complete by.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_computeFromOldString:(NSString *)text1 andNewString:(NSString *)text2 checkLines:(BOOL)checklines deadline:(NSTimeInterval)deadline;

/**
* Do a quick line-level diff on both strings, then rediff the parts for
* greater accuracy.
* This speedup can produce non-minimal diffs.
* @param text1 Old NSString to be diffed.
* @param text2 New NSString to be diffed.
* @param deadline Time when the diff should be complete by.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_lineModeFromOldString:(NSString *)text1 andNewString:(NSString *)text2 deadline:(NSTimeInterval)deadline;

/**
* Split two texts into a list of strings.  Reduce the texts to a string of
* hashes where each Unicode character represents one line.
* @param text1 First NSString.
* @param text2 Second NSString.
* @return Three element NSArray, containing the encoded text1, the
*     encoded text2 and the NSMutableArray of unique strings. The zeroth element
*     of the NSArray of unique strings is intentionally blank.
*/
- (NSArray *)diff_linesToCharsForFirstString:(NSString *)text1 andSecondString:(NSString *)text2;

/**
* Rehydrate the text in a diff from an NSString of line hashes to real lines
* of text.
* @param diffs NSArray of Diff objects.
* @param lineArray NSMutableArray of unique strings.
*/
- (void)diff_chars:(NSArray<Diff *> *)diffs toLines:(NSMutableArray<NSString *> *)lineArray;

/**
* Find the 'middle snake' of a diff, split the problem in two
* and return the recursively constructed diff.
* See Myers 1986 paper: An O(ND) Difference Algorithm and Its Variations.
* @param text1 Old string to be diffed.
* @param text2 New string to be diffed.
* @param deadline Time at which to bail if not yet complete.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_bisectOfOldString:(NSString *)text1 andNewString:(NSString *)text2 deadline:(NSTimeInterval)deadline;

/**
* Given the location of the 'middle snake', split the diff in two parts
* and recurse.
* @param text1 Old string to be diffed.
* @param text2 New string to be diffed.
* @param x Index of split point in text1.
* @param y Index of split point in text2.
* @param deadline Time at which to bail if not yet complete.
* @return NSMutableArray of Diff objects.
*/
- (NSArray<Diff *> *)diff_bisectSplitOfOldString:(NSString *)text1 andNewString:(NSString *)text2 x:(NSInteger)x y:(NSInteger)y deadline:(NSTimeInterval)deadline;

/**
* Determine if the suffix of one CFStringRef is the prefix of another.
* @param text1 First NSString.
* @param text2 Second NSString.
* @return The number of characters common to the end of the first
*     CFStringRef and the start of the second CFStringRef.
*/
- (NSInteger)diff_commonOverlapOfFirstString:(NSString *)text1 andSecondString:(NSString *)text2;

/**
* Do the two texts share a substring which is at least half the length of
* the longer text?
* This speedup can produce non-minimal diffs.
* @param text1 First NSString.
* @param text2 Second NSString.
* @return Five element String array, containing the prefix of text1, the
*     suffix of text1, the prefix of text2, the suffix of text2 and the
*     common middle. Or `nil` if there was no match.
*/
- (NSArray<NSString *> * _Nullable)diff_halfMatchOfFirstString:(NSString *)text1 andSecondString:(NSString *)text2;

/**
* Does a substring of shorttext exist within longtext such that the
* substring is at least half the length of longtext?
* @param longtext Longer NSString.
* @param shorttext Shorter NSString.
* @param index Start index of quarter length substring within longtext.
* @return Five element NSArray, containing the prefix of longtext, the
*     suffix of longtext, the prefix of shorttext, the suffix of shorttext
*     and the common middle.  Or nil if there was no match.
*/
- (NSArray<NSString *> * _Nullable)diff_halfMatchIOfLongString:(NSString *)longtext andShortString:(NSString *)shorttext index:(NSInteger)index;

/**
* Given two strings, comAdde a score representing whether the internal
* boundary falls on logical boundaries.
* Scores range from 5 (best) to 0 (worst).
* @param one First string.
* @param two Second string.
* @return The score.
*/
- (NSInteger)diff_cleanupSemanticScoreOfFirstString:(NSString *)one andSecondString:(NSString *)two;

/**
* Locate the best instance of 'pattern' in 'text' near 'loc' using the
* Bitap algorithm.   Returns NSNotFound if no match found.
* @param text The text to search.
* @param pattern The pattern to search for.
* @param loc The location to search around.
* @return Best match index or NSNotFound.
*/
- (NSInteger)match_bitapOfText:(NSString *)text andPattern:(NSString *)pattern near:(NSInteger)loc;

/**
* Compute and return the score for a match with e errors and x location.
* @param e Number of errors in match.
* @param x Location of match.
* @param loc Expected location of match.
* @param pattern Pattern being sought.
* @return Overall score for match (0.0 = good, 1.0 = bad).
*/
- (double)match_bitapScoreForErrorCount:(NSInteger)e location:(NSInteger)x near:(NSInteger)loc pattern:(NSString *)pattern;

/**
* Increase the context until it is unique,
* but don't let the pattern expand beyond Match_MaxBits.
* @param patch The patch to grow.
* @param text Source text.
*/
- (void)patch_addContextToPatch:(Patch *)patch sourceText:(NSString *)text;

@end

NS_ASSUME_NONNULL_END
