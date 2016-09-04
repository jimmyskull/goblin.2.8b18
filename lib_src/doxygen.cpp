
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, February 2007
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   doxygen.cpp
/// \brief  Several doxygen commands used to generate this documentation


/// \mainpage  GOBLIN Graph library
///
/// \image html  dodekahedron.eps
///
///
/// \section projectScope  Project summary
///
/// Goblin is a full-featured tool chain for handling \ref pageGraphObjects "graphs".
/// The project provides code from the following areas:
/// - <b>Optimization</b>: Nearly all algorithms described in textbooks on graph optimization
///   are implemented.
/// - <b>Layout</b>: The most common models (\ref secOrthogonalDrawing "orthogonal",
///   \ref secLayeredDrawing "layered", \ref secForceDirected "force directed") are
///   supported.
/// - <b>Composition</b>: Especially for planar graphs, a lot of rules are implemented
///   to derive one graph from another.
/// - <b>File import and export</b>: Graphs can be read from Dimacs, Tsplib and Steinlib
///   formats. Layouts can be exported to nearly arbitrary bitmap and canvas
///   formats by implicit use of the fig2dev and the netpbm packages.
/// - <b>Manipulation</b>: Graph incidence structures can be edited and attributes can be
///   assigned to nodes and edges in the graphical front end.
///
/// While all algorithms are implemented in a C++ class library (libgoblin.a),
/// the editor code and partially the file I/O code are written in the Tcl/Tk
/// scripting language and use a Tcl wrapper of the C++ library.
///
/// Users can choose from the following interfaces:
/// - A C++ programming interface (API) described here
/// - A Tcl/Tk programming interface which is less efficient, but sometimes more
///   convenient
/// - A graphical user interface (GUI) to manipulate graphs, but also to run
///   problem solvers and to visualize the (intermediate) results
/// - Solver executables for several optmization problems
///
/// <br>
///
///
/// \section intendedAudience  Intended audience
///
/// Any general software on graph optimization can be considered a loose collection
/// of solvers for more or less related combinatorial problems. Other than for LP
/// solvers, the interface is ample and often requires to choose a particular
/// optimization method which is adequate for a certain class of graph instances.
/// Many applications require a programming interface instead of a graphical
/// user interface. In that sense, Goblin is expert software.
///
/// Originally, the library has been designed to visualize graph algorithms. While
/// it takes special efforts to produce good running examples and snap shots, the
/// final output can be interpreted by every undergraduate student who has been
/// instructed with the algorithm's key features. The GUI has been prepared to
/// fiddle about with algorithms.
///
/// Of course, the library is also intended for practical computations. A general
/// statement on the performance is difficult: Some solvers have been added only
/// for sake of completeness (like max-cut), others can solve large-scale instances
/// (like min-cost flow).
///
/// <br>
///
///
/// \section licenceModel  Licence
///
/// GOBLIN is open source software and licenced by the GNU Lesser Public License
/// (LGPL). That is, GOBLIN may be downloaded, compiled and used for scientific,
/// educational and other purposes free of charge. For details, in particular
/// the statements about redistribution and changes of the source code, observe
/// the LGPL document which is attached to the package.
///
/// <br>
///
///
/// \section projectState  Project state
///
/// The project state permanently toggles between stable and beta: There are
/// frequent functional extensions, and these sometimes require revisions of the
/// internal data structures and even the C++ programming interface.
/// 
/// Here are some more detailed remarks on the development state:
/// - The optimization stuff is very comprehensive and, in general, stable. Future
///   releases will come up with alternative methods and performance improvements
///   rather than solvers for additional problems.
/// - The drawing codes are mostly stable, but several general features are missing
///   (3D support, infinite nodes and edges, free-style display features).
///   The orthogonal methods lack strong compaction rules.
/// - The internal graph representations still do not allow for node hierarchies and
///   arbitrary tags on nodes and edges.
/// - Tracing is supported for many but not for all methods. This functionality
///   is still difficult to control.
///
/// <br>
///
///
/// \section thisDocument  About this document
///
/// This document serves as a reference guide for the C++ core library. For the time
/// being, it describes the API formed by the graph base classes, but skips
/// a lot of internal features such as problem reductions and the LP wrapper.
/// So it is not as comprehensive as the existing latex manual yet, but the latter
/// won't be maintained regularly until this doxygen reference has become stable.
///
/// Two levels of reading are possible: The introductory pages present mathematical
/// defintions, high-level descriptions of algorithms and some instructive examples.
/// In order to view the according doxygen code comments, follow the [See API] links.
///
/// <br>
///
///
/// \section manualContents  Page index
///
/// - Managed data objects
///     - \ref pageGraphObjects
///         - \ref pageGraphSkeleton
///         - \ref pageRegisters
///         - \ref pageSubgraphManagement
///         - \ref pageGraphManipulation
///         - \ref pagePlanarity
///     - Data structures
///         - Attributes and attribute pools
///         - Index sets
///         - Investigators
///         - Containers
///     - Branch and bound
/// - Algorithms
///     - Graph optimization
///         - \ref pageEdgeRouting
///             - \ref pageNetworkFlows
///         - \ref pageConnectivity
///         - \ref pageVertexRouting
///         - \ref pageGraphPacking
///     - \ref pageGraphLayout
///     - \ref pageGraphComposition
///     - \ref pageGraphRecognition
/// - Interfaces
///     - \ref groupObjectExport
///     - \ref groupObjectImport
///     - \ref groupCanvasBuilder
///     - \ref groupTextDisplay
///     - \ref groupTransscript
///     - \ref groupMipReductions


/// \defgroup queryReprAttributes  Representational attributes
/// \defgroup nodeCoordinates      Node coordinates
/// \defgroup specialEntities      Special entities

/// \page pageGraphObjects  Graph objects
///
/// Graphs are objects from discrete geometry which consist of so-called nodes
/// and edges. Graphs may be either undirected (where edges are unordered pairs
/// of nodes), directed (where edges are ordered pairs), or mixed from directed
/// and undirected edges. To this mathematic definition, we refer as the
/// incidence structure or the \ref pageGraphSkeleton "graph skeleton".
///
/// Most graph optimization problems require further input, usually numeric values
/// associated with the graph edges. For the time being, the library does not
/// support arbitrary tags on the graph nodes and edges, but attributes which
/// either assign a value to \em all nodes or to \em all edges.
///
/// There are so-called attribute pools to manage attributes with predefined
/// roles, and to make them persistent:
/// - \ref secReprAttributes
/// - \ref pageRegisters
/// - \ref secNodeCoordinates
/// - Layout data
///
/// All graph objects provide consistent interfaces to retrieve values of the
/// attributes in these pools. The attribute values are either virtual (functional
/// dependent on another graph object) or represented in memory. Register
/// attributes are always represented, even for graph objects which result from
/// a combinatorial problem transformation.
///
/// A graph object is called represented if all of the managed attributes are
/// represented. If the graph skeleton is also represented by data structures,
/// the graph representation is called \em sparse (and \em dense otherwise).
///
/// <br>
///
///
/// \section secGraphSubclasses  Subclass hierarchy
///
/// The distinction between directed, undirected and mixed graphs is reflected
/// by the library class hierarchy. All high level methods are associated with
/// according abstract classes. From the abstract class, a sparse and a dense
/// representation subclass is derived.
///
/// <br>
///
///
/// \subsection secMixedGraphs  Mixed graphs
///
/// The class \ref abstractMixedGraph is the base class for all kinds of graph
/// objects. It implements all layout and optimization methods which do not
/// impose restrictions on the graph skeleton, which can ignore edge orientations
/// or which can handle both the directed and the undirected case.
///
/// \ref abstractMixedGraph "[See abstract base class]"
///
/// \ref mixedGraph "[See sparse implementation]"
///
/// <br>
///
///
/// \subsection secDigraphs  Digraphs (directed graphs)
///
/// The class \ref abstractDiGraph is the base class for all digraph objects.
/// It implements most of the network flow code and everything about
/// \ref secDirectedAcyclic "DAGs".
///
/// \ref abstractDiGraph "[See abstract base class]"
///
/// \ref diGraph "[See sparse implementation]"
///
/// \ref denseDiGraph "[See dense implementation]"
///
/// <br>
///
///
/// \subsection secUndirGraphs  Undirected graphs
///
/// The class \ref abstractGraph is the base class for all undirected graphs.
/// It implements the matching and T-join solvers, and specialized methods
/// for TSP, max-cut and Steiner trees.
///
/// \ref abstractGraph "[See abstract base class]"
///
/// \ref graph "[See sparse implementation]"
///
/// \ref denseGraph "[See dense implementation]"
///
/// <br>
///
///
/// \subsection secBigraphs  Bigraphs (bipartite graphs)
///
/// Bigraphs divide the node set into two parts such that edges run from one
/// part to the other. For several optimization problems, the bipartite case
/// is either trivial (e.g. node colouring, stable set) or at least, there are
/// special algorithms (e.g. matching, edge colouring).
///
/// \ref abstractBiGraph "[See abstract base class]"
///
/// \ref biGraph "[See sparse implementation]"
///
/// \ref denseBiGraph "[See dense implementation]"
///
/// <br>
///
///
/// \section secReprAttributes  Representational attributes
///
/// Representational attributes denote the data which must be added to a plain
/// graph in order to obtain a network programming problem instance. That are so far:
/// - Lower and upper edge capacity bounds which restrict the set of feasible subgraphs
/// - Edge length labels which define linear objective functions
/// - Node demands which define terminal or source node sets
///
/// The edge length labels are special in the following sense: In geometric problem
/// instances, the edge lengths are functional dependent on the node coordinate values
/// irrespective of what is set in the physical edge length attribute.
///
/// \ref queryReprAttributes "[See API]"
///
/// <br>
///
///
/// \section secNodeCoordinates  Node coordinate values
///
/// To the nodes of a represented graph, coordinate values can be assigned.
/// Coordinate values form part of the graph drawing, but also can define the
/// edge length labels. The latter depends on the TokGeoMetric
/// attribute value. Currently, there no distinction between display coordinates
/// and length defining coordinates.
/// 
/// When coordinate values exist, one says that the graph is geometrically embedded.
/// All nodes have the same dimension, and this dimension is either 0 or 2.
/// Three-dimensional embeddings are not well-supported yet since all layout
/// methods produce 2D drawings.
///
/// Coordinate values are also provided for all potential layout points. But note
/// that these coordinate values might be relative to graph node coordinates.
///
/// \ref nodeCoordinates "[See API]"
///
/// <br>
///
///
/// \section relatedGraphObjects  Related topics
///
/// - \ref pageGraphSkeleton
/// - \ref pageRegisters
/// - \ref pageSubgraphManagement
/// - \ref pageGraphManipulation
/// - \ref pagePlanarity
/// - \ref secMapIndices
///
/// \ref index "Return to main page"


/// \defgroup arcOrientations  Arc orientations
/// \defgroup queryIncidences  Node and arc incidences
/// \defgroup nodeAdjacencies  Node adjacencies

/// \page pageGraphSkeleton  Graph skeleton
///
/// The graph skeleton denotes all information defining the incidence structure
/// of a graph:
/// - Start and end nodes of a given arc
/// - A node incidence list for every node
/// - Information which edges are directed
///
/// There is a universal interface to retrieve information about the skeleton.
/// That is, dense graphs also have a logical view of incidence lists, but no
/// according memory representation. Only the skeleton of graphs with a sparse
/// representation can be manipulated.
///
/// <br>
///
///
/// \section secArcOrientations  Arc orientations
///
/// Basically, edges are represented by indices in an interval [0,1,..,m-1].
/// Technically however, every edge (say with index i) has two directions
/// (2i and 2i+1), and most methods expect edge parameters including the arc
/// direction bit. This is independent of whether the whole graph is directed,
/// undirected or mixed.
///
/// Observe that Orientation(2i) and Orientation(2i+1) are the same, and this
/// tells whether the edge i is directed. In that case, Blocking(2i) returns false
/// but Blocking(2i+1) returns true. The latter method is used to generalize
/// optimization methods from directed to mixed graphs (e.g. for minimum spanning
/// arborescence or max-cut). It is not applied by shortest path methods which
/// occasionally operate on implicitly modified digraphs (residual networks).
///
/// \ref arcOrientations "[See API]"
///
/// <br>
///
///
/// \section secQueryIncidences  Node and arc incidences
///
/// From the logical viewpoint, all graph objects - not just those with a sparse
/// representation - own an incidence structure to be investigated by algorithms:
/// - StartNode(i) and EndNode(i) give the two nodes which are connected by the arc
///   i with the identity StartNode(i)==EndNode(i^1).
/// - First(v) gives either NoArc or an arc index such that StartNode(First(v))==v.
/// - Right() can be used to enumerate all incidences of a particular node,
///   including the blocking backward arcs. One has StartNode(Right(i))==StartNode(i).
///
/// For dense graphs, all these method internally calculate node indices from arc
/// indices or vice versa. Sparse graphs have incidence lists represented in memory.
///
/// Node incidence lists cannot only be searched by directly calling First() and
/// Right(). There are proxy objects, called investigators, maintaining an incidence
/// list pointer (i.e. an arc index) for every graph node.
///
/// \ref queryIncidences "[See API]"
///
/// <br>
///
///
/// \section secNodeAdjacencies  Node adjacencies
///
/// In some situations, it is necessary to decide whether two given nodes are
/// adjacent (that is, if the nodes are joined by an edge) or even to know a
/// connecting arc. By default, there is no data structure for this purpose.
/// For dense graphs, a call to Adjacency() calculates the connecting arc only
/// by using the given node indices. In its general implementation, the method
/// uses a hash table which is build on the first call. Observe that this hash
/// table is invalidated by any maipulation of the graph skeleton.
///
/// \ref nodeAdjacencies "[See API]"
///
/// <br>
///
///
/// \section relatedGraphSkeleton  Related topics
///
/// - \ref secManipSkeleton
/// - \ref secManipIncidences
///
/// \ref index "Return to main page"


/// \defgroup distanceLabels  Distance labels
/// \defgroup nodePotentials  Node potentials
/// \defgroup nodeColours  Node colours
/// \defgroup edgeColours  Edge colours
/// \defgroup predecessorLabels  Predecessor labels

/// \page pageRegisters  Registers
///
/// Registers denote a variety of graph attributes which are intended to store
/// algorithmic results persistently. All registers are collected into a single
/// attribute pool, and this pool can be accessed by the return of Registers().
///
/// Opposed to the other graph attributes, registers are available for all graph
/// objects, not just for represented graphs. If a graph is represented, manipulations
/// of its skeleton ensure that the register attributes are updated accordingly.
///
/// <br>
///
///
/// \section secNodeColours  Node colours
///
/// The node colour register assigns integer values to the graph nodes, with
/// the intended value range [0,1,..,n-1] + {NoNode}. The applications are:
/// - Orderings of the node set (e.g. st-numberings)
/// - Partitions of the node set (e.g. clique partitions)
///
/// \ref nodeColours "[See API]"
///
/// <br>
///
///
/// \section secEdgeColours  Edge colours
///
/// The edge colour register assigns integer values to the graph arcs, with
/// the intended value range [0,1,..,2m-1] + {NoArc}. The applications are:
/// - Orderings of the arc set (e.g. Euler cycles)
/// - Partitions of the arc set (e.g. partitions into 1-matchings)
/// - Implicit arc orientations (e.g. feedback arc sets)
/// Other than the subgraph multiplicities, edge colours are stored by plain
/// arrays. So, especially for geometric graph instances, the number of arcs
/// is large relative to the size of memory representation. It is therefore
/// recommended to handle subgraph incidence vectors by edge colours only if
/// non-trivial lower capacity bounds prevent from using the subgraph
/// multiplicities.
///
/// \ref edgeColours "[See API]"
///
/// <br>
///
///
/// \section secPredecessorLabels  Predecessor labels
///
/// The predecessor register assigns arc indices to the graph nodes. More
/// particularly, the predecessor arc of a node is either undefined or points
/// to this node.
///
/// Any directed path encoded into the predecessor labels can be backtracked
/// from its end node without scanning the incidence lists of the intermediate
/// nodes.
/// So, whenever possible, subgraphs are stored by the predecessor register:
/// - Simple directed paths and cycles
/// - Rooted trees
/// - One-cycle trees consisting of a directed cycle and some trees pointing
///   away from this cycle
/// - Any node disjoint union of the listed subgraph types
///
/// \ref predecessorLabels "[See API]"
///
/// <br>
///
///
/// \section secDistanceLabels  Distance labels
///
/// The distance label register maintains the results of shortest path methods.
///
/// \ref distanceLabels "[See API]"
///
/// <br>
///
///
/// \section secNodePotentials  Node potentials
///
/// The node potential register maintains the dual solutions for min-cost network
/// flow problems. That is, network flow optimality can be verified at any time
/// after a flow has been been computed, and later calls to the min-cost flow
/// solver will take advantage of the dual solution.
///
/// \ref nodePotentials "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup subgraphManagement  Subgraph management
/// \defgroup degreeLabels  Node degree labels

/// \page pageSubgraphManagement  Subgraphs
///
/// As the name indicates, subgraph multiplicities encode all kinds of subgraphs
/// and flows, only restricted by the lower and upper capacity bounds.
///
/// Like register attributes, subgraph multiplicities are defined for all graph
/// objects. Other than registers, subgraph multiplicities are represented by
/// arrays only for sparse graphs. For dense graphs, a hash table is used. By
/// that, the memory to store subgraphs grows proportional with the subgraph
/// cardinality. For logical graph instances (e.g. network flow transformations),
/// subgraph multiplicities are not stored separately, but synchronized with the
/// subgraph multiplicities of the original graph.
///
/// \ref subgraphManagement "[See API]"
///
/// <br>
///
///
/// \section secDegreeLabels  Node degree labels
///
/// Degree labels denote the node degrees according to the current subgraph
/// multiplicities. The degree label of a node cumulates the multiplicity of the
/// arcs incident with this node. Actually there are different fuctions Deg(),
/// DegIn() and DegOut(), the first to count the undirected edges, the second to
/// count the entering directed arcs, and the third to count the emanating
/// directed arcs.
///
/// The degree labels are indeed represented in memory. For the sake of efficiency,
/// the following is implemented:
/// - The degree labels are only generated with a call to Deg(), DegIn() or DegOut().
///   But then all node degrees are computed in one pass.
/// - Once generated, with every change of a subgraph multiplicity, the degree labels
///   of the respective end nodes are adjusted.
/// - The degree labels can be disallocated again to save this book keeping operations
///   by calling ReleaseDegrees(). This in particular happens when the subgraph
///   is reset by using InitSubgraph().
/// For the time being, the degree labels of a sparse graph are corrupted by
/// graph skeleton manipulations.
///
/// \ref degreeLabels "[See API]"
///
/// <br>
///
///
/// \section relatedSubgraphManagement  Related topics
///
/// - \ref pageNetworkFlows
/// - \ref secMatching
///
///
/// \ref index "Return to main page"



/// \defgroup nodePartitions  Node partitions
/// \ingroup graphObjects


/// \defgroup manipSkeleton  Manipulating the graph skeleton
/// \defgroup manipIncidences  Manipulating node incidence orders

/// \page pageGraphManipulation  Graph manipulation
///
/// A graph object can be manipulated only if it is represented, and manipulation
/// methods are addressed to its representational object. Register atttributes
/// are not attached to representational objects, but are available for every
/// graph object. That is, register attribute value can be set and modified for
/// logical graph instances also.
///
/// Both sparse and dense representations allow changes of the representational
/// attributes (capacity bounds, arc lengths, node demands) but the skeleton can
/// be manipulated only for sparsely represented graphs.
///
/// A pointer to the representational object is returned by the method
/// abstractMixedGraph::Representation() which is defined for every graph object,
/// but returns a NULL pointer for non-represented graph objects.
///
///
/// <br>
///
///
/// \section secManipSkeleton  Manipulating the graph skeleton
///
/// In order to manipulate the skeleton of a sparse graph G, the representational
/// object of G must be dereferenced and type-casted like this:
///
/// <code> static_cast< \ref sparseRepresentation *>(G.Representation()) </code>.
///
/// Sparse representational objects admit the following operations:
/// - Arc insertions and deletions
/// - Node insertions and deletions
/// - Reversion of the (implicit) arc orientations
/// - Swap the indices of a pair of nodes or of a pair of arcs
/// - Arc contraction to a single node
/// - Identification of a pair of nodes
///
/// Also, the graph layout code is partially implemented by this class.
///
///
/// \subsection secManipCasePlanar  Planarity issues
///
/// In principle, these operations maintain planar representations. For the arc
/// insertions and the node identification operations, this can only work if the
/// two (end) nodes are on a common face, practically the exterior face. If the common
/// face exist, but is not the exterior face, abstractMixedGraph::MarkExteriorFace()
/// must be called to make it temporarily exterior (This sets the First() indices
/// after that new arcs are inserted into the incidence list).
///
///
/// \subsection secInvalidation  Invalidation rules
///
/// The following operations invalidate existing node and arc indices:
/// - sparseRepresentation::DeleteArc()
/// - sparseRepresentation::DeleteNode()
/// - sparseRepresentation::DeleteArcs()
/// - sparseRepresentation::DeleteNodes()
///
/// That is, after the call DeleteArc(a), arc indices might address different
/// data before and after the arc deletion. This does not only concern the
/// specified index a. The method DeleteNode() causes even more trouble, since
/// node deletions imply the deletion of all incident arcs.
///
/// It is possible to delay these index invalidation effects by using the method
/// sparseRepresentation::CancelArc() instead of DeleteArc(a). Doing so, the
/// edge is deleted from its end nodes incidence lists (so that subsequent graph
/// search won't traverse it), but all arc attributes remain unchanged. After the
/// process of arc deletions has been completed, a call of DeleteArcs() will
/// pack all attributes.
///
/// The CancelNode() and DeleteNodes() methods work pretty similar as CancelArc()
/// and DeleteArc(). But observe that programming sparseRepresentation::CancelNode(v)
/// implies a CancelArc() call for every arc incident with v, and that DeleteNode(v)
/// and DeleteNodes() imply a call of DeleteArcs().
///
/// \ref manipSkeleton "[See API]"
///
/// <br>
///
///
/// \section secManipIncidences  Manipulating node incidence orders
///
/// A node incidence list enumerates all arcs with a given start node. The lists
/// are defined by the method abstractMixedGraph::Right() and are essentially
/// circular. But there is a special list entry abstractMixedGraph::First() to
/// break the circular list at.
///
/// Manipulating the incidence order can be done by two means:
/// - Calling sparseRepresentation::SetRight(): This cuts a circular list in three
///   pieces and relinks the pieces such that a single circular list is maintained.
///   This is a tricky operation if several concurrent calls are necessary, since
///   the relative order of the arcs passed to SetRight() is restricted.
/// - Calling sparseRepresentation::ReorderIncidences() takes an array of arc
///   indices which specifies for every arc index a1 in [0,1,..,2m-1] another
///   arc index which is set as the return value of Right(a1). So the procedure
///   assigns all right-hand arc indices simultaneously. It is not so difficult
///   to handle, but efficient only if the incidence orders can be assigned in
///   one pass.
///
/// Observe that in a \ref pagePlanarity "planar representation", the incidence
/// lists of exterior nodes (node on the exterior face) start with two exterior
/// arcs (there might be more than two candidates if the node is a cut node).
/// So in the planar case, rather than setting the First() indices directly, it
/// is recommended to call abstractMixedGraph::MarkExteriorFace() which adjusts
/// the planar representation consistently such that the left-hand region of the
/// passed arc becomes the exterior face.
///
/// \ref manipIncidences "[See API]"
///
/// <br>
///
///
/// \section relatedManipulation  Related topics
///
/// - \ref pageGraphSkeleton
/// - \ref secPlanarEmbedding
/// - \ref secSeriesParallel
///
/// \ref index "Return to main page"


/// \defgroup planarRepresentation  Planar representation
/// \defgroup planarEmbedding  Planarity recognition and combinatorial embedding

/// \page pagePlanarity  Planarity
///
/// A drawing of a graph is called plane if no pair of edges crosses each other.
/// A graph is planar if it admits a plane drawing. This definition of planar
/// graphs is fairly independent of the allowed class of edge visual representations
/// (straight lines, Jordan curves or open polygones).
///
/// Planar graphs often allow efficient optimization where the general case is
/// hard to solve (e.g. maximum cut). Usually, codes on planar graphs do not
/// use explicit drawings but only the respective clockwise ordering of the
/// node incidence lists. If the incidence lists are ordered as in some plane
/// drawing, this is called a planar representation.
///
/// <br>
///
///
/// \section secPlanarRepresentation  Planar representation
///
/// Consider the situation where a sparse graph is given, and one wants to know
/// if the given node incidence lists form a planar representation and
/// occasionally to generate an according plane drawing. In that case, it does
/// not help to perform a planarity test since the graph might be planar but this
/// particular incidence lists do not form a planar representation.
/// Depending on the application, it might be also unwanted to compute a
/// planar representation from scratch.
///
/// For the described purpose, one calls ExtractEmbedding() which determines the
/// left-hand face indices of all arcs in the plane case. Later on, these indices
/// can be retrieved by abstractMixedGraph::Face(). And arcs with the same face
/// index are on the counter-clockwise boundary of this face in any compliant
/// plane drawing.
///
/// Assigning left-hand face indices means to implicitly generate the dual graph.
///
/// \ref planarRepresentation "[See API]"
///
/// <br>
///
///
/// \section secPlanarEmbedding  Planarity recognition and embedding
///
/// A planarity test decides whether a graph is planar or not without having a
/// combinatorial embedding, but only an arbitrary order in the node incidence
/// lists. In addition, a plane embedding code reorders the node incidence lists
/// to form a planar representation.
/// 
/// One of the most famous theorems on graphs states that every non-planar graph
/// has a K_5 or a K_3_3 minor, that are subgraphs obtained by subdividing the
/// edges of a K_5 (a complete graph on five node) or of a K_3_3 (a complete bigraph
/// with three nodes in each part).
/// 
/// While generating the dual graph / the faces is a certificate fora proper
/// planar representation, the mentioned minors allow to verify a negative result
/// of the plane embedding code. Note that only the Hopcroft/Tarjan code can
/// derive such minors, and does only when this is explicitly required.
///
/// \ref planarEmbedding "[See API]"
///
/// <br>
///
///
/// \section relatedPlanarity  Related topics
///
/// - \ref secPlanarDrawing
/// - \ref secSeriesParallel
/// - \ref secPlanarComposition
///
/// \ref index "Return to main page"


/// \defgroup dataStructures        Data structures
/// \ingroup dataObjects
/// \defgroup attributes            Attributes and attribute pools
/// \ingroup dataStructures
/// \defgroup indexSets             Index sets
/// \ingroup dataStructures
/// \defgroup investigators         Investigators
/// \ingroup dataStructures
/// \defgroup containers            Containers
/// \ingroup dataStructures
/// \defgroup contiguousMemory      Contiguous memory containers
/// \ingroup containers
/// \defgroup nodeBasedContainers   Node based containers
/// \ingroup containers
/// \defgroup priorityQueues        Priority queues
/// \ingroup contiguousMemory
/// \defgroup setFamilies           Set families
/// \ingroup contiguousMemory

/// \defgroup objectDimensions      Object dimensions
/// \ingroup dataObjects
/// \defgroup classifications       Object classifications
/// \ingroup dataObjects


/// \defgroup shortestPath      Shortest path methods
/// \defgroup minimumMeanCycle  Minimum mean cycles
/// \defgroup eulerPostman      Eulerian cycles and supergraphs

/// \page pageEdgeRouting  Edge routing problems
///
/// \section secShortestPath  Shortest path methods
///
/// When applying the shortest path solver, one probably thinks of simple paths
/// (paths not repeating any node). In fact, all paths / cycles / trees are
/// represented by the predecessor labels which naturally imposes a restriction
/// to simple paths or cycles. The label correcting codes technically determine
/// walks (which possibly repeat arcs) rather than simple paths and fail if both
/// notations diverge for this graph instance.
///
/// The method to compute shortest a path between two given nodes s and t heavily
/// depends on the configuration of arc length labels and orientations. The
/// following list shows the available codes in decreasing order of computational
/// efficiency, and it is recommended to apply the first method which is feasible
/// for the given instance:
/// - In the case of coinciding, non-negative length labels, a breadth first search is sufficient
/// - If the graph is directed acyclic, apply the DAG search method
/// - If all length labels are non-negative, apply the Dijkstra method
/// - If all negative length arcs are directed, apply a label-correcting method
/// - If the graph is undirected, apply the T-join method (with T={s,t})
///
/// The general NP-hard setting with negative length cycles is not handled by the library.
///
/// Most library methods determine a shortest path tree routed at a given source
/// node s rather than a single shortest path between a given node pair s and t,
/// and this tree is exported by the predecessor labels.
/// Specifying a target node t is relevant for the BFS and the Dijkstra method
/// since the seach can be finished prematurely, when t is reached the first time.
/// Only the T-join method constructs an exports a single st-path.
///
/// The following figure shows an undirected graph and a shortest path between
/// the red filled nodes. Since negative length edges exist, only the T-join
/// method is formally applicable (undirected edges are interpreted as antiparallel
/// arc pairs in the other codes). If one inspects the graph thoroughly, one finds
/// a negative-length cycle. In general, the T-join method will be aborted when
/// extracting the st-path from the T-join subgraph in such a situation. But if
/// the minimum T-join is nothing else than an st-path, it is a shortest st-path:
/// \image html  tjoin2.eps
///
/// \ref shortestPath "[See API]"
///
/// <br>
///
///
/// \section secMinimumMeanCycle  Minimum mean cycles
///
/// Several min-cost flow methods search for negative length cycles in the so-called
/// residual network and push the maximum possible augment of flow on these cycles.
/// If the cycles are arbitrary, the number of iterations cannot be bounded polynomially.
/// But one obtains a strongly polynomial running time if the cycles are <em> minimum
/// mean cycles</em>, that is, if the total edge length divided by the number of edges
/// is minimal. Actually, this min-cost flow method has poor performance compared
/// with the primal network simplex code. The core minimum mean cycle code is provided
/// only to serve other applications in the future.
///
/// \ref minimumMeanCycle "[See API]"
///
/// <br>
///
///
/// \section secEulerPostman  Eulerian cycles and supergraphs
///
/// An Eulerian cycle is a cycle which meets every edge exactly once and which
/// traverses every edge in a non-blocking direction. Graphs which admit such
/// cycles are called Eulerian graphs. Due to the interpretation of arc capacities
/// as arc multiplicities, and the fact that Eulerian cycles are stored like an
/// ordering of the edge set, computing an Eulerian cycle requires that the upper
/// capacity bounds are all one. If this is not the case, use
/// sparseRepresentation::ExplicitParallels() for preprocessing.
///
/// The following is a so-called Sierpinksi triangle. This is a recursively
/// defined, infinite graph, and Eulerian at any recurrency level. An Eulerian
/// cycle is displayed both by the edge labels and the edge colours:
/// \image html  euler4.eps
///
/// The well-known Chinese postman problem asks for an increase of the upper
/// capacity bounds (interpreted as arc multiplicities) such that the graph
/// becomes Eulerian and the length of an Eulerian cycle is minimal. In the
/// undirected setting (and only then), it is equivalent to determine a maximum
/// length Eulerian subgraph. The library handles the directed and the undirected
/// case, but not the generalized NP-hard problem for mixed graphs.
/// \image html  postman1.eps
///
/// For the above graph, with the shown length labels and unit capacities, the
/// following picture shows a maximum Eulerian subgraph in bold face, and a
/// minimum length Eulerian augmentation:
/// \image html  postman2.eps
///
/// \ref eulerPostman "[See API]"
///
/// <br>
///
///
/// \section relatedEdgeRouting  Related topics
///
/// - \ref pageNetworkFlows
/// - \ref secMatching
///
///
/// \ref index "Return to main page"


/// \defgroup flowLowLevel  Low level operations
/// \defgroup maximumFlow   Maximum st-flow
/// \defgroup minCostFlow   Minimum cost st-flow, b-flow and circulation

/// \page pageNetworkFlows  Network flows
///
/// By a <em>pseudo-flow</em>, one usually denotes a subgraph multiplicity vector
/// bounded by given lower and upper capacity vectors. In the library, this is
/// only true for directed arcs. For undirected edges, lower capacity bounds have
/// to be zero, the absolute flow value is bounded by the upper capacity bound,
/// and a negative flow value denotes a flow running in the implicit backward
/// direction.
///
/// The standard network flow models also use the node demand labels and the
/// concept of node divergence: The divergence of a node v is the flow sum of
/// arcs entering v minus the flow sum of arcs leaving v. A b-flow is a
/// pseudo-flow where all node divergences match the respective node demands
/// (say: the nodes are balanced). So in the standard notation, the node
/// demand vector is abbreviated by b.
///
/// <br>
///
///
/// \section secFlowLowLevel  Low level operations
///
/// There is a technique which occurs in nearly all network flow methods, called
/// \em augmentation. This means pushing flow along a given path, increasing the
/// subgraph multiplicities of forward arcs and decreasing the subgraph
/// multiplicities of arcs which occur in backward direction. For a single arc,
/// this is exactly what abstractDiGraph::Push() does to the subgraph multiplicities
/// (the procedure also updates the node degree labels).
///
/// There are two obvious applications of the augmentation technique:
/// - The augmenting path is a cycle and the (residual) edge length of this cycle
///   is negative. Then augmentation will decrease the overall costs of the flow.
/// - The start node of the augmenting path is oversaturated and the end node
///   is undersaturated. In that case, augmentation will reduce the node imbalances.
///
/// In both situations, the methods abstractDiGraph::FindCap() and abstractDiGraph::Augment()
/// apply. The first determines the minimum amount of flow which can be pushed along
/// a path, and the second method actually modifies the subgraph multiplicities and
/// the node degree labels.
///
/// Augmenting paths are usually determined by searching the so-called <em>residual
/// network</em> which is implicitly defined by the functions abstractMixedGraph::ResCap()
/// and abstractMixedGraph::RedLength(). The latter function depends on an array
/// of node potentials (usually stored in the so-named register) and is called in
/// the weighted setting only.
///
/// \ref flowLowLevel "[See API]"
///
/// <br>
///
///
/// \section secMaximumFlow  Maximum st-flow and feasible b-flow
///
/// The maximum st-flow problem distinguishes two nodes s (the source) and t (the
/// sink or target) from the other graph nodes. An st-flow denotes a pseudo-flow
/// which is balanced at all nodes other than s and t. It is maximum if the flow
/// divergence at s is minimized (and maximized at t).
///
/// The following shows a maximum st-flow where s is the left-most node, and t
/// is the right-most node in the drawing. A minimum cut is indicated by the
/// node colours, and the node labels of the white left-hand nodes denote the
/// distance from source in the residual graph:
/// \image html  maxflow4.eps
///
/// A feasible b-flow (a b-flow which satisfies the capacity bounds) can be
/// determined in the same time complexity order as a maximum st-flow. This is
/// achieved by identifying the divergent nodes and then transforming to a
/// maximum st-flow problem instance.
///
/// \ref maximumFlow "[See API]"
///
/// <br>
///
///
/// \section secMinCostFlow  Minimum cost st-flow, b-flow and circulation
///
/// The min-cost flow solver is one of the library cornerstones for several resaons:
/// - Min-cost flow is indeed one of the most important graph optimization models.
///   An important library internal application is in the weighted matching code.
/// - The solver includes various alternative methods, some are intended for practical
///   computations, others have been added only for didactic purposes.
/// - The primal network simplex, the cost-scaling and the capacity scaling method
///   are performant enough to solve problem instances with 10000s of edges.
/// - Min-cost flow perfectly illustrates the application of linear programming
///   duality to graph optimization models.
///
/// The min-cost flow dual variables in terms linear programming are called <em>
/// node potentials </em> as usual, and are stored by the \ref secNodePotentials
/// "register attribute" with this name. All min-cost flow methods return with
/// an optimal flow and with optimal node potentials. When the solver is restarted,
/// it dependends on the particular method and on the intermediate manipulations,
/// if and how the method draws benefit of maintaining the node potentials:
/// - The Klein and the minimum mean cycle canceling method operate on the
///   original edge lengths and so do not depend on the node potentials.
///   In principle, the Klein method computes node potentials in every iteration
///   from scratch.
/// - The cost scaling method initializes the scaling parameter with the
///   most-negative reduced arc length. If nothing has changed since the last
///   solver call, the method detects this.
/// - The capacity scaling method starts with an arbitrary pseudo-flow and searches
///   a restricted residual network depending on the scaling parameter and the reduced
///   edge length. Again, If nothing has changed since the last solver call, no
///   flow augmentations or potential updates occur.
/// - The primal network simplex method starts with growing a spanning tree from
///   the edges with zero reduced length. Even if nothing has changed since the
///   last solver call, degenerate pivots can occur. Explicit application of an LP
///   solver is similar.
///
/// \ref minCostFlow "[See API]"
///
/// <br>
///
///
/// \section relatedNetworkFlows  Related topics
///
/// - \ref secShortestPath
/// - \ref pageEdgeRouting
/// - \ref pageSubgraphManagement
/// - \ref secMatching
///
///
/// \ref index "Return to main page"


/// \defgroup groupMinCut  Minimum cuts
/// \defgroup groupComponents  Connectivity components
/// \defgroup groupPlanarAugmentation  Planar connectivity augmentation

/// \page pageConnectivity  Connectivity and minimum cuts
///
/// \section secMinCut Minimum Cuts
///
/// In the standard setting, a minimum edge cut denotes a node set bipartition of
/// an undirected graph such that a minimum multiplicity sum of the edges with
/// end nodes in different parts results. The library also deals with a couple
/// of problem variations:
/// - The directed setting, better: The general setting of mixed graphs. Only those
///   directed arcs count which are unblocking from the left-hand to the right-hand
///   side.
/// - The setting where a left-hand node and / or a right-hand node are fixed
/// - The setting with restricted node capacities (abstractMixedGraph::NodeConnectivity()
///   and abstractMixedGraph::StrongNodeConnectivity()). If all node capacities are
///   virtually infinite, this is the same as computing edge cuts.
///
/// To all methods, the upper capacity bounds apply as the edge multiplicities and,
/// for the methods dealing with restricted node capacities, the node demands are
/// interpreted as the node capacities.
///
/// The derived cuts a exported by the node colour register with the following
/// interpretation:
/// - Colour index 0 stand for cut nodes
/// - Colour index 1 stand for left-hand nodes
/// - Colour index 2 stand for right-hand nodes
///
/// \ref groupMinCut "[See API]"
///
/// <br>
///
///
/// \section secComponents  Connected components
///
/// Given a fixed connectivity number and characteristic, one might be interested
/// in a connectivity test and maximal connected subgraphs.
///
/// For the edge connectivity and the strong edge connectivity case, the library
/// includes methods to partition the node set such that all partial induced
/// subgraphs are maximally connected. These methods formally return whether the
/// graph object is connected and, if not, export the connected components by
/// the node colour register. The respective codes for 1-edge connectivity,
/// 2-edge connectivity and strong 1-edge connectivity run in linear time. The
/// codes for high connectivity orders iterate on the min-cut methods, and hence
/// are polynomial but not really efficient.
///
/// Concerning vertex connectivity, a partition into edge disjoint blocks is
/// possible (and implemented) only in the low order cases: For 1-connectivity
/// and strong 1-connectivity, the respective edge connectivity methods apply.
/// The depth-first search method abstractMixedGraph::CutNodes() returns the
/// 2-blocks (the edge-disjoint, maximal biconnected subgraphs) by the edge
/// colour register, and the cut nodes by the node colour register. This
/// procedure
///
/// Starting with k=3, the maximal k-connected subgraphs (called <em>k-blocks</em>)
/// are in general not edge disjoint as the following graph illustrates:
/// \image html  connect8.eps
///
/// Herein, the green nodes form a cutting pair, and the green edge is contained
/// in both 3-blocks. There is the concept of SPQR-trees to represent the 3-blocks,
/// but this is out of the library scope.
///
/// \ref groupComponents "[See API]"
///
/// <br>
///
///
/// \section secPlanarAugmentation  Planar connectivity augmentation
///
/// Especially planar drawing algorithms depend on certain connectivity levels
/// of the input graph. To meet these requirements, the library contains methods
/// to augment planar represented graphs such that the resulting graph is also
/// planar, the derived planar representation can be reduced to the original
/// planar representation and the ouput is connected.
///
/// Here are some caveats:
/// - The methods do not apply to general (non-planar) graphs
/// - The methods do not even apply to implicitly planar graphs, only when a
///   planar representation is at hand
/// - The augmentation methods also require a certain level of connectivity.
///   That is, abstractMixedGraph::Triangulation() requires a biconnected planar
///   graph, and abstractMixedGraph::PlanarBiconnectivityAugmentation() requires
///   a connected graph.
/// - Both methods do not derive minimal augmentations, and do not even achieve a
///   fixed approxiation ratio. Even if each single augmentation step would be
///   optimal, step-by-step application would not.
///
/// Although the library code is very elaborate, there are also theoretical
/// limitations: The planar biconnectivity augmentation problem is NP-hard.
/// For the planar triconnectivity augmentation of biconnected graphs, it is
/// not even known if a polynomial time method is possible.
///
/// Dropping the planarity requirement admits linear time augmentation methods,
/// but no such library code is available.
///
/// \ref groupPlanarAugmentation "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup spanningTree  Spanning tree methods
/// \defgroup steiner       Steiner trees
/// \defgroup tsp  Travelling salesman

/// \page pageVertexRouting  Vertex routing problems
///
/// \section secSpanningTree  Spanning tree methods
///
/// A tree is a connected, cycle free graph. A tree subgraph which meets all
/// graph nodes is called a spanning tree. One say, a tree is rooted at the node
/// r if for every node v, the tree path connecting r and v is non-blocking.
/// A tree rooted at node r is sometimes called an r-arborescence.
///
/// Trees might be represented by either the predecessor labels, the subgraph
/// multiplicities or the edge colours. Generally, the predecessor label
/// representation is used. Only the Kruskal method differs: At an intermediate
/// step of adding a tree edge, it does not know the arc orientation in the
/// final rooted tree. The subgraph representation is used instead of the edge
/// colours since sparse subgraphs of complete graphs are represented more
/// economically.
///
/// The travelling salesman solver uses the concept of a one-cycle tree exposing
/// an (arbitrary but fixed) node r: This denotes a spanning tree of the nodes
/// other than r, adding the two minimum length edges incident with r. That yields
/// a cycle through r and, when this cycle is contracted to a single node, a
/// spanning tree rooted at the artificial node. Minimum one-cycle trees
/// constitute lower bounds on the length of a Hamiltonian cycle.
///
/// The following shows a set of nodes in the Euclidian plane, a minimum one
/// cycle tree exposing the upper left node, and a minimum spanning tree rooted
/// at the same node:
/// \image html  euclidian.eps
///
/// \ref spanningTree "[See API]"
///
/// <br>
///
///
/// \section secSteinerTree  Steiner trees
///
/// The discrete Steiner tree problem is defined on (potentially sparse) graphs
/// and distinguishs between terminal and Steiner graph nodes. In that setting,
/// a Steiner tree denotes a tree or arborescence which is rooted at some terminal
/// node and which spans all other terminal nodes. The Steiner nodes are spanned
/// only if they form shortcuts between terminal nodes.
///
/// \ref steiner "[See API]"
///
/// <br>
///
///
/// \section secTsp  Travelling salesman
///
/// A Hamiltonian cycle (or simply a tour) is a cycle which meets every graph
/// node exactly once. The travelling salesman problem asks for a Hamiltonian
/// cycle of minimum length.
///
/// As computing an optimal tour is NP-hard, the library includes construction
/// heuristics, methods to compute lower bounds on the minimum tour length, and
/// a branch & bound scheme.
///
/// In the following example, a graph, the length labels and an acoording
/// minimum length tour are shown. The optimality can be concluded from the
/// fact that this tour is also a minimum reduced length one-cycle tree (for
/// the upper left node). The reduced length labels and the node potentials
/// applied for length reduction are also displayed:
/// \image html  tsp8.eps
///
/// \ref tsp "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup stableSet     Stable sets and cliques
/// \defgroup colouring     Colouring
/// \defgroup maxCut        Maximum edge cuts
/// \defgroup feedbackSets  Feedback arc sets
/// \defgroup treePacking   Tree packing
/// \defgroup matching      Matchings & T-Joins

/// \page pageGraphPacking  Packings, coverings and partitions
///
/// \section secStableSet  Stable sets and cliques
///
/// A stable set (or independent node set) is a node set without adjacent node
/// pairs. A clique is a node set such that each pair of contained nodes is
/// adjacent. A vertex cover is a node set which is incident with all edges.
///
/// The solver for the maximum stable set problem uses branch & bound with a
/// fixed clique partition for bounding. Minimum vertex covers are obtained
/// by taking the complementary node set of a maximum stable set. Maximum
/// cliques are obtained by computing maximum stable sets in complementary
/// graphs. The latter is inefficient if the original graph is very sparse.
///
/// The following diagram shows an independent positioning of queens on a chess
/// board. In the underlying graph, nodes are adjacent if they are connected by
/// a straight line:
/// \image html  queens.eps
///
/// \ref stableSet "[See API]"
///
/// <br>
///
///
/// \section secColouring  Colouring
///
/// A node colouring is an assignment of the node colour register such that the
/// end nodes of every edge have different colours. In other words, this denotes
/// a partition of the node set into stable sets.
///
/// Bipartite graphs mean 2-colourable graphs. Probably the most famous theorem in
/// graph theory states that every planar graph can be coloured with at most 4
/// different colours. The library comes up with a 5-colouring scheme for planar
/// graphs. For the general setting, no efficient approximating method is available.
///
/// Observe that any clique size is a lower bound on the chromatic number (the
/// minimum number of colour classes). If both numbers are equal and a maximum
/// clique is given in advance (by the node colour register!), the node colour
/// enumeration scheme will find a minimal colouring with some luck. Otherwise,
/// only little effort is spent on computing a clique for dual bounding and it
/// is most likely that the enumeration must be interrupted to obtain the heuristic
/// solution found so far.
///
/// Similar to node colouring is the task of partitioning the node set into a
/// minimum number of cliques. In the library, this problem is solved by searching
/// for node colourings in the complementary graph. Needless to say that this is
/// inefficient if the original graph is very sparse. If the graph is triangle
/// free, however, clique partitioning is equivalent with maximum 1-matching, and
/// this can be solved efficiently.
///
/// The following illustrates the clique cover problem. It is obvious that the
/// displayed cover is minimal, since the graph does not include any 4-cliques
/// (and this in turn follows from the fact that the graph is outerplanar):
/// \image html  dual4.eps
///
/// \ref colouring "[See API]"
///
/// <br>
///
///
/// \section secMaxCut  Maximum edge cuts
///
/// The maximum cut problem asks for a bipartition of the node set such that
/// the total length of cross edges is maximized where arc capacities are
/// interpreted as multiplicities. Note the difference with the min-cut solver
/// which does not consider arc lengths, only capacities.
///
/// The solver also applies to directed and even mixed graphs. To this end,
/// let the bipartition distinguish into left-hand and right-hand nodes.
/// The goal is to maximize the length of non-blocking arcs in the left-to-right
/// direction.
///
/// The following is a directed unit length and capacity example where the
/// green nodes are left-hand and the red nodes are right-hand nodes:
/// \image html  maxcut1.eps
///
/// \ref maxCut "[See API]"
///
/// <br>
///
///
/// \section secFeedbackSets  Feedback arc sets
///
/// A feedback arc set of a digraph is an arc set such that reverting these arcs
/// results in a \ref secDirectedAcyclic "directed acyclic graph". Computing a
/// minimum feedback arc set is NP-hard, but near-optimal solutions are used
/// in the \ref secLayeredDrawing "layered drawing" approach to obtain readable
/// drawings where the most arcs run in the same direction.
///
/// There is no elaborate branch & bound scheme for this problem, but only a 1/2
/// approximation method which is pretty analugue of the max-cut approximation.
///
/// \ref feedbackSets "[See API]"
///
/// <br>
///
///
/// \section secTreePacking  Tree packing
///
/// The maximum tree packing problem asks for a maximum cardinality set of edge
/// disjoint spanning trees rooted at a common node, say r. It is well-known that
/// this maximum equals the minimum cardinality of a directed cut with the node
/// r on the left-hand side.
///
/// In principle, the library method for maximum tree packing also applies to
/// the capacitated extension of this problem. But the procedure is only weakly
/// polynomial, and the packing can be saved completely to the edge colour register
/// only in the standard setting since a non-trivial capacity bound C means that
/// this arc can be in C arborescences. The latter limitation can be overcome
/// by using the extended API which allows to generate the arborescences step by
/// step.
///
/// The procedure is little performant, and suited rather to illustrate the min-max
/// relationship with minimum r-cut on small examples. Today, there is no method
/// for the undirected analogue or even the extension to mixed graphs.
///
/// \ref treePacking "[See API]"
///
/// <br>
///
///
/// \section secMatching  Matchings & T-Joins
///
/// In the literature, a b-matching is a subgraph of an undirected graph such
/// that for all nodes, the subgraph node degree does not exceed the node demand,
/// Graph edges may occur in the b-matching with arbitrary multiplicity. A
/// b-matching is called perfect if the subgraph node degrees and the node demands
/// are equal.
///
/// There is also a standard notation of f-factors which differ from perfect
/// b-matchings by that one does not allow for edge multiplicities. The perfect
/// b-matching and the f-factor notation meet in the case of 1-factors respectively
/// perfect 1-matchings which are subgraphs where every node is incident with
/// exactly one edge. And 1-matchings are payed most attention in the literature,
/// usually referring to matchings rather than 1-matchings. An example of a
/// minimum cost 1-factor is given in the following figure:
/// \image html  optmatch12.eps
///
/// For the library matching solver, edges may be assigned with arbitrary upper
/// capacity bounds so that both f-factors and b-matchings are covered. It is possible
/// to determine maximum matchings and minimum cost perfect matchings, even with
/// specifying differing lower and upper node degree bounds.
///
/// For an undirected graph and some node set T with even cardinality, a T-join
/// is a subgraph such that exactly the nodes in T have odd degree. This model
/// covers several other optimization problems:
/// - Shortest paths problem instances with negative length edges
/// - The Chinese postman problem (to extend a graph to an Eulerian graph)
/// - The 1-matching problem
///
/// While the first two problems are indeed solved in the library by application
/// of a T-join method, the 1-matching problem cannot be solved this way since
/// it self forms part of the T-join method. When optimizing T-joins explicitly,
/// the odd nodes (terminal nodes) are specified by the node demand labels.
///
/// The following figure shows a subgraph with two odd nodes. In fact, it is a
/// minimum T-join for this odd node set. This example shows that a T-join for a
/// 2-element node set T is a path only in certain circumstances. To this end,
/// a sufficient condition is that no negative length cycles exist:
/// \image html  tjoin9.eps
///
/// \ref matching "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup groupLayoutBasic  Low level procedures
/// \defgroup groupLayoutModel  Layout models
/// \defgroup groupLayoutFilter  Display filters
/// \defgroup groupArcRouting  Arc routing
/// \defgroup groupLayoutCircular  Circular drawing methods
/// \defgroup groupForceDirected  Force directed layout
/// \defgroup groupPlanarDrawing  Planar drawing algorithms
/// \defgroup groupOrthogonalDrawing  Orthogonal drawing algorithms
/// \defgroup groupOrthogonalPlanar  Planar orthogonal drawing
/// \defgroup groupOrthogonalTree  Orthogonal tree drawing
/// \defgroup groupLayeredDrawing  Layered Drawing

/// \page pageGraphLayout  Graph layout
///
/// \section secLayoutModels  Layout models
///
/// The <em>layout model</em> determines many but not all of the display
/// parameters: For example, colours, edge widths and text labels are assigned
/// independently of the layout model, whereas node and arc shape modes are
/// layout model dependent.
///
/// Any transition from one layout model to another requires to call
/// \ref abstractMixedGraph::Layout_ConvertModel() which does some kind of
/// garbage collection for the old layout points and sets the correct display
/// parameters for the new model. When calling a layout method, this is done
/// implicitly. There may be various drawing methods supporting the same layout
/// model!
///
/// In the very basic setting, the <em>straight line model</em>, graph drawing
/// just means to position the graph nodes in the Euclidian plane. But this only
/// works with simple graphs (graphs without parallel edges and loops). Node and
/// arc labels are placed according to certain global rules. It is obvious that
/// such rules produce readable drawings without overlaps only in special cases.
///
/// The are several drawing methods which produce straight line drawings, for
/// example placement of all nodes on a circle or exposing a predecessor tree.
/// The resulting drawings may be input to a
/// \ref secForceDirected "force directed placement (FDP) method". The latter
/// do not always produce straight line drawings but call
/// \ref sparseRepresentation::Layout_ArcAlignment() in order to expose possible
/// parallel edges.
///
/// The more elaborate layout models require to distinguish between graph nodes
/// and <em>layout points</em>. The latter are addressed by node indices ranged
/// in an interval [0,1,..,n+ni-1]. All graph nodes are mapped to layout points,
/// called <em>node anchor points</em>. The additional layout points can be
/// grouped as follows:
/// - <em>Arc label anchor points</em> which denote absolute arc labels positions
/// - <em>Arc bend nodes</em> which denote also absolute positions. When arcs are
///   displayed by open polygones, the bend nodes are traversed. In case of smooth
///   curves, it depends on the graphical tool kit if the bend node positions are
///   interpolated or only approximated (the regular case).
/// - <em>Arc port nodes</em>
/// - Points to determine the graph node display size. At the time, this is
///   feasible only for the visibility representation model.
///
/// For the time being, graph node indices and the respective node anchor point
/// indices coincide, that is <code>NodeAnchorPoint(v)==v</code>.
///
/// \ref groupLayoutModel "[See API]"
///
/// <br>
///
///
/// \section secArcRouting  Arc routing
///
/// The display of any arc is determined by the following layout nodes:
/// - An arc label anchor point given by \ref abstractMixedGraph::ArcLabelAnchor()
/// - A sequence of bend nodes enumerated by \ref abstractMixedGraph::ThreadSuccessor()
/// - Occasionally, two port nodes on the boundary of the arc end nodes visualizations.
///
/// Depending on the applied layout model, these nodes are either implicit or
/// represented by a layout node index and respective coordinates:
/// - In the most simple case, the straight line drawing of simple graphs,
///   no bend nodes exist. The arc label anchor points and the port nodes are
///   implicit and determined by the display of graph nodes.
/// - For all available orthogonal drawing methods, the number of bend nodes is
///   3 or less. In high-degree models, port nodes are explicit, and are implicit
///   in models which admit a maximum node degree of 4 or less.
///
/// Provided that the number of bend nodes is generally bounded, use
/// \ref abstractMixedGraph::GetBendNodes() to write the bend node sequence of a
/// specified arc to aa array, including the port nodes. The result depends on the
/// arc orientation bit. If the number of bend nodes is not known in advance, the
/// sequence of bend nodes of any forward arc a (an even arc index!) is obtained
/// by this recurrency rule:
///
/// <code>
/// bendNode[0] = PortNode(a);
///
/// for (i=0;bendNode[i]!=NoNode;++i) {
///    bendNode[i+1] = ThreadSuccessor(bendNode[i]);
/// }
/// </code>
///
/// If the layout model uses explicit port nodes, those are the extremal
/// points in the bend node sequence, namely <code>bendNode[0]</code> and
/// <code>bendNode[i-1]</code>.
///
/// Manipulation of arc routings is somewhat tricky since, currently, bend nodes
/// cannot be assigned independently from arc label anchor points. Use the most
/// high-level operation \ref sparseRepresentation::InsertBendNode() which still
/// has some side effects: It will implicitly allocate and position an arc label
/// anchor point if it does not exist in advance.
///
/// \ref groupArcRouting "[See API]"
///
/// <br>
///
///
/// \section secPlanarDrawing  Planar straight-line drawing
///
/// Basically, planar drawing means to create a drawing of a planar graph
/// without any edge crossings. In fact, all planar drawing methods depend on a
/// particular planar representation, that is a clockwise ordering of incidence
/// lists according to a virtual plane drawing. Before drawing a planar graph,
/// it is necessary to determine such a planar representation! If no exterior
/// arc has been specified in advance, an exterior face with a maximum number of
/// nodes is set up.
///
/// The library comes up with a method for convex drawing of triconnected planar
/// graphs. This method has been be extended to arbtrirary planar graphs by making
/// the input graph triconnected, then applying the convex drawing method and
/// mapping back the node placement to the original graph. The final drawing
/// probably has non-convex faces even if the input graph is triconnected, since
/// graphs are actually triangulated. It is recommended either to apply the convex
/// method directly or to use the restricted
/// \ref secForceDirected "force directed placement (FDP) method" for post-precessing.
///
/// The following is a convex drawing of the dodecahedron. The good resolution
/// of area and angles stems from the regularity of the input graph:
/// \image html  dodekahedronConvex.eps
///
/// \ref groupPlanarDrawing "[See API]"
///
/// <br>
///
///
/// \section secCircularDrawing  Circular drawing and outerplanarity
///
/// The basic method of placing all nodes on a circle only depends on a particular
/// clock-wise node order. This may be specified explicitly by the node colour
/// register (with arbitrary tie-breaking for nodes with equal colour indices).
/// It is also possible to use the predecessor arc labels, with the special intent
/// to expose Hamiltonian cycles. In the general case, the method backtracks on
/// predecessor arcs as long as possible and places the traversed nodes, and then
/// selects a further unmapped node for backtracking.
///
/// There is another specialization of the circular method which travels around
/// the exterior face of a planar graph. It is restricted to the class of
/// \em outerplanar graphs. This denotes all graphs which have a plane drawing
/// with all nodes on the exterior face. The outerplanar \ref secPlanarRepresentation
/// representation must be given in advance.
///
/// The next two figures show the same outerplanar graph, starting with the single
/// circular drawing:
/// \image html  outerplanar.eps
///
/// Some biconnected outerplanar graphs also admit an \em equilateral drawing.
/// In that drawing, all nodes of a single interior face form a regular polyhedron.
/// Observe that all equilateral drawings of a graph are congruent. A plane drawing
/// results only in special cases. The original intent was to obtain paper models
/// of Platonian and Archimedian polyhedra. For this purpose, there is another
/// method to cut triconnected planar graphs along the edges of some spanning tree.
/// What follows, is an example of such a paper model, obtained for the octahedron:
/// \image html  equilateral.eps
///
/// \ref groupLayoutCircular "[See API]"
///
/// <br>
///
///
/// \section secForceDirected  Force directed layout
///
/// Force directed drawing is a post-opimization strategy for straight line
/// drawings. It models attracting and repelling forces between nodes and,
/// occasionally, between node-edge pairs. One iterates for a node placement
/// which brings all forces into equilibriance. The classical spring embedder
/// moves all nodes simultaneously, but the much faster GEM code moves only
/// one node at a time. There is a version which restricts the node movements
/// such that nodes do not cross any edges. The application to plane drawings
/// is obvious, but the method can be used for general graphs, in order to
/// maintain a so-called mental map.
///
/// \ref groupForceDirected "[See API]"
///
/// <br>
///
///
/// \section secOrthogonalDrawing  Orthogonal drawing algorithms
///
/// An orthogonal drawing is a drawing where all graph nodes are displayed by
/// rectangles and all arcs are displayed by polygones running in a rectangular
/// grid. The display points at which an arc is attached to its end nodes, are
/// called port nodes. For sake of readability, overlapping nodes and crossings
/// of edges with non-incident nodes are excluded.
///
/// There are various specializations of this general orthogonal grid drawing
/// model. Most refer to the output of particular drawing algorithms, and usually
/// restrict the input graph somehow (planarity, connectivity, loops, parallels,..):
/// - The small node model is limited to graphs where node degrees are at most 4.
///   In this model, no explicit port nodes are handled. There are specialized
///   methods for planar graphs and for binary trees, but also a method for the
///   non-planar setting.
/// - The edge display can be restricted to a certain number of bends. No more
///   than three bends per edge are used in the library methods.
/// - In the 1-visibility representation model, edges are bend-free and running
///   only in vertical directions. The nodes only have horizontal extent. Thus
///   this model is naturally restricted to planar graphs.
/// - In the Kandinsky model, two grids are used. Nodes are displayed as squares
///   of equal size in a coarse grid which is specified by the nodeSep parameter.
///   When post optimization is turned off, every edge has exactly one bend node.
///   There are methods for the general and for the planar setting, for trees and
///   for redrawing a graph, in order to maintain the so-called <em>mental map</em>.
///
/// For the small-node model and for the Kandiski model, but not for the
/// visibility-representation model, post-optimization procedure exist. By that,
/// grid lines can be merged and nodes can move from one grid line to an adjacent
/// line, but only if this reduces the number of bend nodes or the total edge
/// length. The effects are tremendous, but the final drawings are still far from
/// optimal, especially in terms of the required area.
///
/// The following is a small node drawing of a dodekahedron. The library does
/// not provide an exact method for bend minimization, but it is easy to see that
/// this particular drawing is bend-minimal:
/// \image html  dodekahedron.eps
///
/// This figure has not been generated automatically, but by using the
/// interactive Kandinski method. Although the procedure does not mind plane
/// or small-node drawings, it is helpful for post-processing such drawings!
///
/// \ref groupOrthogonalDrawing "[See API]"
///
/// <br>
///
///
/// \subsection secOrthogonalPlanar  Planar case
///
/// The library admits three strategies to obtain plane orthogonal drawings,
/// quite different to orchestrate:
/// - Visibility representation: For a given biconnected planar (represented)
///   graph, bipolar orientations of the input graph and its dual graph are
///   computed. The node coordinates are determined by the distance labels in
///   these digraphs. If the input graph is not biconnected, it is augmented.
///   There is an optional post-processing mode, reducing the node sizes to the
///   minimum of what is possible without re-routing the arcs. For graphs where
///   the node degree is bounded by 3, a small node drawing can be obtained.
/// - 4-orthogonal drawings: The method also applies to non-planar graphs. It
///   checks for the existence of a planar representation and occasionally adapts
///   itself to the planar setting. However, the input graph must be biconnected
///   (Augmentation does not make sense here!)
/// - Kandinski drawings: The so-called <em>staircase method</em> starts with an
///   exterior edge and inserts the other nodes step by step, according to a
///   <em>canonical ordering</em> which is available for triconnected planar
///   graphs only. In each intermediate step, the overall shape is a half-square
///   triangle, and node are inserted on the diagonal side. There is an interface
///   which extends the method to non-triconnected graphs by triangulation of
///   the input graph.
///
/// The following figure has been obtained from applying the staircase method
/// to the dodecahedron without any post-proceesing. In this example, the post
/// optimization code would reduce the area by a factor of 10.3 (to the same
/// area as in the previous drawing), and the number of bends by a factor
/// of 5.0 (two more bends than in the optimal solution above):
/// \image html  dodekahedronStaircase.eps
///
/// An example of a visibility representation can be found with the description
/// of \ref secSeriesParallel "series-parallel graphs".
///
/// \ref groupOrthogonalPlanar "[See API]"
///
/// <br>
///
///
/// \subsection secOrthogonalTree  Orthogonal tree drawing
///
/// An HV-drawing of a binary tree is a plane straight line drawing such that
/// the child nodes of any node are placed right-hand or below of the parent
/// node. It is a rather small class of graph to which the method can be applied.
/// But the results are appealing for that symmetries are exposed, as in the
/// following drawing of a depth 4 regular binary tree:
/// \image html  binTree.eps
///
/// Trees with high degree nodes can be drawn in the 1-bent Kandinski model.
/// As in the binary case, parent nodes appear in the upper left corner of the
/// spanned subtree bounding box. Other than for HV-trees, child nodes are always
/// displayed below of the parent nodes.
///
/// \ref groupOrthogonalTree "[See API]"
///
/// <br>
///
///
/// \section secLayeredDrawing  Layered Drawing
///
/// In a layered drawing, the node set is partitioned into independent sets
/// (the layers), each layer is assigned to a horizontal grid line,
/// and bend nodes are placed wherever edges would cross grid lines in a
/// straight line drawing. This first phase of the strategy is called vertical
/// placement. Then in the horizontal placement phase, the nodes are arranged
/// within their layers. Hereby, original graph nodes and bend nodes are treated
/// almost the same.
///
/// The optimization goals in the vertical placement step are the aspect ratio
/// and the total edge span (this denotes the number of required bend nodes).
/// If the graph is directed or mixed, it is also intended to orient the arcs
/// top-down. The goals in the horizontal placement are to prevent from edge
/// crossings and to maximize the slopes of the edge segments.
///
/// Although it is a couple of more or less independent tools, the layered
/// drawing code is called through a single interface function. If LAYERED_DEFAULT
/// is specified, the graph is drawn from scratch. One should not expect plane
/// or even upward-plane drawings for planar graphs. The main reason is that the
/// crossing minimization code does not question the layer assignment done in
/// advance.
///
/// Frankly, the rules which code is applied under which conditions are complicated:
/// - By specifying LAYERED_FEEDBACK, the the top-down arc orientations are
///   explicitly computed (In case of directed acyclic graphs, all arcs will point
///   downwards). If this option is not specified, the orientations must be either
///   given in advance, are implicated by an existing layer assignment or by the
///   given drawing (depending on the other rules for vertical placement).
/// - When specifying LAYERED_EDGE_SPAN, nodes are assigned to layers by applying
///   the exact edge span minimization code.
/// - When specifying LAYERED_COLOURS, all nodes with finite colour index are
///   forced to the respective layer. If the layering is not complete, the edge
///   span minimization code method is applied.
/// - In any case, arcs spanning more than one layer, are subdivided by bend nodes.
///   This completes the vertic placement phase.
/// - By specifying LAYERED_SWEEP, the crossing minimization code is applied.
///   This performs local optimization between two adjacent layers, sweeping
///   top-down and bottom-up again, until no further improvement is possible.
///   Crossing minimization is the performance critical part of the layered
///   drawing method. After that step, the relative order of nodes in each layer
///   is fixed.
/// - When specifying LAYERED_FDP, a one dimensional force directed method does
///   the horizontal node alignment. The node moves are either unlimited or
///   preserve the relative node order set by the crossing minimization code.
/// - By specifying LAYERED_ALIGN, an LP model is applied to maximize the slopes
///   of the edge segments.
///
/// In order to refine a given plane drawing, one should specify LAYERED_EDGE_SPAN
/// and a method for horizontal placement, but skip the crossing minimization step.
/// If no implicit top-down orientation has been computed in advance and
/// delivered by the edge colours, the orientation is chosen as for the current
/// drawing. It is not required that the previous drawing is in the layered model,
/// but on the other hand, nodes might move vertically.
///
/// The following shows an edge span minimum upward drawing of upward planar
/// digraph. This has been obtained in the default mode. In order to obtain a
/// plane drawing, one node must be shifted manually to the next layer. But even
/// then, the crossing minimization code would not clean up all edge crossings
/// automatically:
/// \image html  unix.eps
///
/// \ref groupLayeredDrawing "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup groupGraphComposition  Graph composition
/// \defgroup groupDagComposition  DAG composition
/// \ingroup groupDirectedAcyclic
/// \defgroup groupPlanarComposition  Planar composition
/// \defgroup groupMapIndices  Mapping node and arc indices back

/// \page pageGraphComposition  Graph composition
///
/// \section secDagComposition  DAG composition
///
/// In a directed graph, the arc set can be considered a relation of the graph
/// nodes. According to that, the \ref transitiveClosure "transitive closure"
/// of a digraph is the digraph with the same node set and with arcs uv if
/// there is a directed uv-path in the original graph. If the original digraph
/// is acyclic, the transitive closure defines a partially ordered set.
///
/// Of more practical importance is the reverse construction principle: An arc
/// uv is called transitive if there is a directed uv-path with intermediate
/// nodes. The \ref intransitiveReduction constructor omits all transitive
/// arcs of the original digraph. Observe that this subgraph is well-defined
/// only if the original digraph is acyclic and simple. The so-called <em> Hasse
/// diagram </em> of a DAG is obtained by taking the intransitive reduction and
/// computing a layered drawing of it.
///
/// The following shows a directd acyclic graph, with the red arcs forming its
/// intransitive reduction:
/// \image html  intransitiveReduction.eps
///
/// \ref groupDagComposition "[See API]"
///
/// <br>
///
///
/// \section secPlanarComposition  Planar composition
///
/// The library provides several methods to generate planar graphs from other
/// planar graphs. It is required that the original graph comes with a planar
/// representation, and the new graph is generated with a respective planar
/// representation. The composition methods generalize from construction
/// principles for regular polyhedra.
///
/// Most important is the \ref dualGraph construction. This takes the original
/// regions as the node set, connecting nodes if the original faces share
/// an edge. Actually, the edges are mapped one-to-one. The \ref directedDual
/// constructor works similar but assigns arc orientations acoording to the
/// orientation of the original arc.
///
/// Observe that the dual of the dual is basically the original graph, provided
/// that the graph is connected. In the directed case, the arc orientations are
/// reverted.
///
/// A potential drawing of the original graph is mapped, but the quality of
/// the resulting drawing is currently limited by the dual node representing
/// the exterior face of the original graph (A special treatment of infinite
/// nodes is missing). The follwoing shows an outer-planar graph and its dual
/// graph, with the exterior face displayed in the center:
/// \image html  dual4_compound.eps
///
/// Other constructors (\ref vertexTruncation and \ref planarLineGraph) replace the
/// original nodes by faces. The constructor \ref facetSeparation maintains the
/// original faces and dupliates the original nodes for every adjacent face.
/// Observe that all constructors yield node-regular graphs:
/// - In \ref vertexTruncation, every node has degree 3
/// - In \ref planarLineGraph, every node has degree 4
/// - In \ref facetSeparation with the facetSeparation::ROT_NONE option,
///   every node has degree 4
/// - In \ref facetSeparation with the facetSeparation::ROT_LEFT or the
///   facetSeparation::ROT_RIGHT option, every node has degree 5
///
/// In order to obtain the full set of Platonian and Archimedian polyhedra, the
/// constructor method polarGrid comes into play which is very flexible:
/// - The tetrahedron is a polar grid with 1 rows, 3 columns and 1 pole
/// - The octahedron is a polar grid with 1 rows, 4 columns and 2 poles
/// - The icosahedron is a polar grid with 2 rows, 4 columns, triangular faces and 2 poles
/// - The dodekahedron is the dual of an icosahedron
/// - Prisms are polar grids with 2 rows, square faces and no poles (and the
///   cube the prism with 4 columns)
/// - Antiprisms are polar grids with 2 rows, triangular faces and no poles
///
/// The following shows a polar grid with 4 rows, 11 columns and a single pole
/// (to obtain this drawing, polarGrid::POLAR_CONE must be specified):
/// \image html  polarGrid.eps
///
///
///
/// \ref groupPlanarComposition "[See API]"
///
/// <br>
///
///
/// \section secMapIndices  Mapping node and arc indices back
///
/// After a graph object has been derived from another data object, and computations
/// have been done on the derived graph instance, it is often required to map back
/// the computational results to the original graph. For this goal, two methods
/// abstractMixedGraph::OriginalNode() and abstractMixedGraph::OriginalArc() are provided.
///
/// There is no global rule of what the index returned by abstractMixedGraph::OriginalNode(v)
/// represents. It may be a node index, an arc index with or without the arc
/// direction bit or any other entity which can be addressed by an index.
///
/// Backtransformations are maintained only if abstractMixedGraph::OPT_MAPPINGS
/// is specified to the respective constructor method. Not all concrete classes
/// support this feature at the time!
///
/// \ref groupMapIndices "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup groupSeriesParallel  Series-parallel graphs
/// \ingroup manipIncidences
/// \defgroup groupDirectedAcyclic  Directed acyclic graphs
/// \ingroup shortestPath

/// \page pageGraphRecognition  Graph recognition
///
/// \section secSeriesParallel   Series-parallel graphs
///
/// A series-parallel graph is a graph which can be obtained from a single edge
/// by recursive application of two production rules: (1) Replacing any edge by
/// parallel edges with the same end nodes as the original edge, and (2)
/// subdividing an edge. It is possible to extend this concept to digraphs and
/// mixed graphs.
///
/// Recognition of series-parallel (di)graphs means to revert the production
/// process and to represent it by a so-called decomposition tree. The tree is used
/// for the combinatorial embedding (ordering of the incidence list) and by the
/// drawing method.
///
/// It is pretty obvious that series-parallel graphs are planar, even
/// upward-planar. When series-parallel graphs are embedded, the resulting
/// incidence orders are planar. When series-parallel graphs are drawn,
/// visibility representations result:
/// \image html  seriesParallel5.eps
///
/// \ref groupSeriesParallel "[See API]"
///
/// <br>
///
///
/// \section secDirectedAcyclic  Directed acyclic graphs (DAGs)
///
/// A DAG is a digraph not containing directed cycles or, equivalently,
/// which has a <em> topologic ordering </em> (a node enumeration such that for
/// every forward arc, the start node has a smaller number than the end node).
/// And by the second characterization, DAGs can be recognized in linear time.
///
/// The importance of DAGs is that minimum and maximum length paths can be also
/// computed in linear time, irrespective of the edge length signs: Predecessor
/// arcs and distance labels are just assigned in the topologic node order.
///
///
/// \ref groupDirectedAcyclic "[See API]"
///
/// <br>
///
///
/// \ref index "Return to main page"


/// \defgroup branchAndBound  Branch & bound
/// \ingroup algorithms
/// \defgroup mixedInteger  (Mixed) integer programming
/// \ingroup branchAndBound


/// \defgroup groupObjectExport  Exporting objects to files
/// \defgroup groupObjectImport  Importing objects from file
/// \defgroup groupCanvasBuilder  Building canvasses
/// \defgroup groupTextDisplay  Text display
/// \defgroup groupTransscript  Output to the transscript channel
/// \defgroup groupMipReductions  Problem reductions to MIP

/// \page pageFileExport
///
///
/// \ref index "Return to main page"

// The following page has no reference but copies the background image
/// \page pageDummy
/// \image html  wall3.jpg
