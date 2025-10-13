let opcodes = [
  "PARTITION_BEGINLOOPCONTROL",
  "loopInit",
  "loopIncrement",
  "jumpIfRepeatDone",
  "PARTITION_BEGINEXPRESSIONS",
  "fetch",
  "fetchFrom",
  "loadVar",
  "setVar",
  "loadVarFrom",
  "loadArrayAt",
  "push",
  "add",
  "DEBUGEXPRESSION",
  "PARTITION_BEGINSTATEMENTS",
  "loopJump",
  "joinString",
  "clone",
  "jumpIf",
  "jump",
  "motionGoto",
  "motionGlideto",
  "motion_glideIteration",
  "motionTurnright",
  "motionTurnleft",
  "motionMovesteps",
  "motionPointindirection",
  "motionPointtowards",
  "motionSetx",
  "motionChangexby",
  "motionSety",
  "motionChangeyby",
  "DEBUGSTATEMENT",
  "stop"
];

let opcodesMap = Object.fromEntries(
  opcodes.map((name) => [
    name,
    { index: null, dataTransformers: [] }
  ])
).pipe((obj) => {
  Object.keys(obj).forEach((name) => {
    obj[name].index = opcodes.indexOf(name);
  });
  return obj;
});

console.log(opcodesMap);
