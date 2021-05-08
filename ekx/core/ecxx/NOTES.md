Entity - is just linear generated integer identifier, stable
Component Handle ID - index to data-store


Method #1 (current)
- On remove: move and pop_back for data and handle-to-entity arrays, patch entity-to-handle sparse-array
- Data stored without "holes" 
Component Handle ID - not stable because of remove

Method #2
- How to check data index is valid? Free list indices could be used
- Data ID is stable

## base components
- Allocators: we should track all allocations inside ECXX
- Assertions: we should be sceptical and guard all places and contracts
- Array: simple dynamic array
- SparseArray: to map entity index to component index
- SmallArray: simple static fixed-capacity array with variable size
