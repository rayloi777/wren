class Component {
  type { "" }
  floatCount { 0 }
  intCount { 0 }
  boolCount { 0 }
  stringCount { 0 }
  
  toFloats() { [] }
  toInts() { [] }
  toBools() { [] }
  toStrings() { [] }
  
  fromFloats(vals) {}
  fromInts(vals) {}
  fromBools(vals) {}
  fromStrings(vals) {}
  
  static newEmpty() { this.new() }
}
