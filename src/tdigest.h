#pragma once
#include <stdlib.h>

/**
 * Adaptive histogram based on something like streaming k-means crossed with Q-digest.
 * The implementation is a direct descendent of MergingDigest
 * https://github.com/tdunning/t-digest/
 * 
 * Copyright (c) 2018 Andrew Werner, All rights reserved.
 *
 * The special characteristics of this algorithm are:
 *
 * - smaller summaries than Q-digest
 *
 * - provides part per million accuracy for extreme quantiles and typically &lt;1000 ppm accuracy for middle quantiles
 *
 * - fast
 *
 * - simple
 *
 * - easy to adapt for use with map-reduce
 */

#define MM_PI 3.14159265358979323846

typedef struct node
{
  double mean;
  double count;
} node_t;

struct td_histogram
{
  // compression is a setting used to configure the size of centroids when merged.
  double compression;

  double min;
  double max;

  // cap is the total size of nodes
  int cap;
  // merged_nodes is the number of merged nodes at the front of nodes.
  int merged_nodes;
  // unmerged_nodes is the number of buffered nodes.
  int unmerged_nodes;

  double merged_count;
  double unmerged_count;

  node_t nodes[];
};

typedef struct td_histogram td_histogram_t;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
 * Allocate the memory, initialise the t-digest, and return the histogram as output parameter.
 * @param compression The compression parameter. 
 * 100 is a common value for normal uses. 
 * 1000 is extremely large. 
 * The number of centroids retained will be a smallish (usually less than 10) multiple of this number.
 * @return the histogram on success, NULL if allocation failed.
 */
  td_histogram_t *td_new(double compression);

  /**
 * Frees the memory associated with the t-digest.
 *
 * @param h The histogram you want to free.
 */
  void td_free(td_histogram_t *h);

  /**
 * Reset a histogram to zero - empty out a histogram and re-initialise it
 *
 * If you want to re-use an existing histogram, but reset everything back to zero, this
 * is the routine to use.
 *
 * @param h The histogram you want to reset to empty.
 *
 */
  void td_reset(td_histogram_t *h);

  /**
 * Adds a sample to a histogram.
 *
 * @param val The value to add.
 * @param weight The weight of this point.
 */
  void td_add(td_histogram_t *h, double val, double weight);

  /**
 * Re-examines a t-digest to determine whether some centroids are redundant.  If your data are
 * perversely ordered, this may be a good idea.  Even if not, this may save 20% or so in space.
 *
 * The cost is roughly the same as adding as many data points as there are centroids.  This
 * is typically &lt; 10 * compression, but could be as high as 100 * compression.
 * This is a destructive operation that is not thread-safe.
 * 
 * @param h The histogram you want to compress.
 *
 */
  void td_compress(td_histogram_t *h);

  /**
 * Merges all of the values from 'from' to 'this' histogram.  
 *
 * @param h "This" pointer
 * @param from Histogram to copy values from.
 */
  void td_merge(td_histogram_t *h, td_histogram_t *from);

  /**
 * Returns the fraction of all points added which are &le; x.
 *
 * @param x The cutoff for the cdf.
 * @return The fraction of all data which is less or equal to x.
 */
  double td_cdf(td_histogram_t *h, double x);

  /**
   * Returns an estimate of the cutoff such that a specified fraction of the data
   * added to this TDigest would be less than or equal to the cutoff.
   *
   * @param q The desired fraction
   * @return The value x such that cdf(x) == q;
   */
  double td_quantile(td_histogram_t *h, double q);

  /**
 * Returns the current compression factor.
 *
 * @return The compression factor originally used to set up the TDigest.
 */
  int td_compression(td_histogram_t *h);

  /**
   * Returns the number of points that have been added to this TDigest.
   *
   * @return The sum of the weights on all centroids.
   */
  double td_size(td_histogram_t *h);

  /**
 * Returns the number of centroids being used by this TDigest.
 *
 * @return The number of centroids being used.
 */
  int td_centroid_count(td_histogram_t *h);

  /**
 * Get minimum value from the histogram.  Will return __DBL_MAX__ if the histogram
 * is empty.
 *
 * @param h "This" pointer
 */
  double td_min(td_histogram_t *h);

  /**
 * Get maximum value from the histogram.  Will return __DBL_MIN__ if the histogram
 * is empty.
 *
 * @param h "This" pointer
 */
  double td_max(td_histogram_t *h);

#ifdef __cplusplus
}
#endif