import { expect, test } from "bun:test";
import { flow } from "./flow.js";

// FIXME: bun does not have a fake timer API yet
test("flow.sleep", async () => {
  const before = process.hrtime.bigint();
  await flow["flow.sleep"]([50]);
  const after = process.hrtime.bigint();

  const diff = after - before;
  // Allow some drift
  expect(diff).toBeGreaterThan(40000000n);

  expect(flow["flow.sleep"](["foo"])).rejects.toThrow();
  expect(flow["flow.sleep"]([])).rejects.toThrow();
  expect(flow["flow.sleep"]([-1])).rejects.toThrow();
});
