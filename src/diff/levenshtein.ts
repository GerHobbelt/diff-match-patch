import {type Diff, DiffType} from './diff.js'

/**
 * Compute the Levenshtein distance; the number of inserted, deleted or
 * substituted characters.
 * @param diffs - Array of diff tuples.
 * @returns Number of changes.
 * @internal
 */
export function levenshtein(diffs: Diff[]): number {
  let leven = 0
  let insertions = 0
  let deletions = 0

  for (let x = 0; x < diffs.length; x++) {
    const op = diffs[x][0]
    const data = diffs[x][1]
    switch (op) {
      case DiffType.INSERT:
        insertions += data.length
        break
      case DiffType.DELETE:
        deletions += data.length
        break
      case DiffType.EQUAL:
        // A deletion and an insertion is one substitution.
        leven += Math.max(insertions, deletions)
        insertions = 0
        deletions = 0
        break
      default:
        throw new Error('Unknown diff operation.')
    }
  }
  leven += Math.max(insertions, deletions)
  return leven
}
