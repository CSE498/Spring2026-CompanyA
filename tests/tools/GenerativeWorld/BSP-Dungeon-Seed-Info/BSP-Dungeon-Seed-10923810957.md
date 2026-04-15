# BSP Dungeon Generation Dump (Given The BSP is generated with seed 1)
# Data information generated from BSP-Dungeon's TreeParser() function, Formatted with help from ChatGPT


###### Some clarification #######
The number next to the node represent the hierarchy in the tree. The highest (#) value of the node (ex. Node #) is the root node
and the descending (#) of the Nodes represent the depth level of node. Nodes of the same (#) value are on the same depth level.

## High Level Abstraction representation of the tree 
Container 
# BSP Tree (Clean View)

# Total Leaf + Node Count in BSP Tree
# Leaf: 15
# Tree: 29


------------
Name Container
x 0
y 0
width 150
height 100
left_child, [Index]: 1
right_child, [Index]: 8
------------
Name 20
x 0
y 0
width 55
height 100
left_child, [Index]: 2
right_child, [Index]: 3
------------
Name 19
x 0
y 0
width 55
height 31
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 19
x 0
y 31
width 55
height 69
left_child, [Index]: 4
right_child, [Index]: 7
------------
Name 18
x 0
y 31
width 55
height 42
left_child, [Index]: 5
right_child, [Index]: 6
------------
Name 17
x 0
y 31
width 55
height 22
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 0
y 53
width 55
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 0
y 73
width 55
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 20
x 55
y 0
width 95
height 100
left_child, [Index]: 9
right_child, [Index]: 14
------------
Name 19
x 55
y 0
width 95
height 26
left_child, [Index]: 10
right_child, [Index]: 13
------------
Name 18
x 55
y 0
width 63
height 26
left_child, [Index]: 11
right_child, [Index]: 12
------------
Name 17
x 55
y 0
width 32
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 87
y 0
width 31
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 118
y 0
width 32
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 19
x 55
y 26
width 95
height 74
left_child, [Index]: 15
right_child, [Index]: 20
------------
Name 18
x 55
y 26
width 52
height 74
left_child, [Index]: 16
right_child, [Index]: 19
------------
Name 17
x 55
y 26
width 52
height 51
left_child, [Index]: 17
right_child, [Index]: 18
------------
Name 16
x 55
y 26
width 52
height 24
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 16
x 55
y 50
width 52
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 55
y 77
width 52
height 23
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 107
y 26
width 43
height 74
left_child, [Index]: 21
right_child, [Index]: 22
------------
Name 17
x 107
y 26
width 43
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 107
y 53
width 43
height 47
left_child, [Index]: 23
right_child, [Index]: 24
------------
Name 16
x 107
y 53
width 43
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 16
x 107
y 73
width 43
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&