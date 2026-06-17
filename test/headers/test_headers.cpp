// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include <spaix/Config.hpp>                  // IWYU pragma: keep
#include <spaix/DataIterator.hpp>            // IWYU pragma: keep
#include <spaix/DataNode.hpp>                // IWYU pragma: keep
#include <spaix/DataPlacement.hpp>           // IWYU pragma: keep
#include <spaix/EntryIterator.hpp>           // IWYU pragma: keep
#include <spaix/Iterator.hpp>                // IWYU pragma: keep
#include <spaix/LinearInsertion.hpp>         // IWYU pragma: keep
#include <spaix/LinearSplit.hpp>             // IWYU pragma: keep
#include <spaix/QuadraticSplit.hpp>          // IWYU pragma: keep
#include <spaix/Queries.hpp>                 // IWYU pragma: keep
#include <spaix/RTree.hpp>                   // IWYU pragma: keep
#include <spaix/RTree.ipp>                   // IWYU pragma: keep
#include <spaix/SideChooser.hpp>             // IWYU pragma: keep
#include <spaix/SplitSeeds.hpp>              // IWYU pragma: keep
#include <spaix/StaticVector.hpp>            // IWYU pragma: keep
#include <spaix/TreeRange.hpp>               // IWYU pragma: keep
#include <spaix/detail/DatEntryType.hpp>     // IWYU pragma: keep
#include <spaix/detail/DirectoryNode.hpp>    // IWYU pragma: keep
#include <spaix/detail/Index.hpp>            // IWYU pragma: keep
#include <spaix/detail/NodePointerEntry.hpp> // IWYU pragma: keep
#include <spaix/detail/attributes.hpp>       // IWYU pragma: keep
#include <spaix/detail/distribute.hpp>       // IWYU pragma: keep
#include <spaix/detail/entry.hpp>            // IWYU pragma: keep
#include <spaix/detail/power.hpp>            // IWYU pragma: keep
#include <spaix/heterox/Comparisons.hpp>     // IWYU pragma: keep
#include <spaix/heterox/Operations.hpp>      // IWYU pragma: keep
#include <spaix/heterox/Point.hpp>           // IWYU pragma: keep
#include <spaix/heterox/Rect.hpp>            // IWYU pragma: keep
#include <spaix/heterox/detail.hpp>          // IWYU pragma: keep
#include <spaix/homox/Comparisons.hpp>       // IWYU pragma: keep
#include <spaix/homox/Operations.hpp>        // IWYU pragma: keep
#include <spaix/homox/Point.hpp>             // IWYU pragma: keep
#include <spaix/homox/Rect.hpp>              // IWYU pragma: keep
#include <spaix/search/Everything.hpp>       // IWYU pragma: keep
#include <spaix/search/Exactly.hpp>          // IWYU pragma: keep
#include <spaix/search/Touching.hpp>         // IWYU pragma: keep
#include <spaix/search/Within.hpp>           // IWYU pragma: keep
#include <spaix/types.hpp>                   // IWYU pragma: keep

#ifdef __GNUC__
__attribute__((const))
#endif
int
main()
{
  return 0;
}
