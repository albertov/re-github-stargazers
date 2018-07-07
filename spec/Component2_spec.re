open Jest;

Enzyme.configureEnzyme(Enzyme.react_16_adapter());

let setup = (~greeting="Test", ()) =>
  Enzyme.shallow(<Component2 greeting />);



describe("Component2", () => {
  open Expect;

  test("contains greeting when show", () => {
    let greeting = "what's up";
    let wrapper = setup(~greeting, ());
    let expectedNode = <span>(ReasonReact.string(greeting))</span>;
    expect(Enzyme.contains(expectedNode, wrapper)) |> toBe(true)
    });

  test("does not contain greeting when not shown", () => {
    let greeting = "what's up";
    let wrapper = setup(~greeting, ());
    let unExpectedNode = <span>(ReasonReact.string(greeting))</span>;
    EnzymeExtra.modifyState (wrapper, s => {...s, Component2.show: false});
    expect(Enzyme.contains(unExpectedNode, wrapper)) |> toBe(false);
    });
  }
);