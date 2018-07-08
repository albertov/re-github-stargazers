/***
 * `modifyState` modifies a wrapped component's internal state.
 * 
 * I was expecting to find this utility in bs-enzyme but didn't
 */
let modifyState = (wrapper, modify) => {
    let jState = Enzyme.jsState(wrapper);
    jState##reasonState #= (modify (jState##reasonState));
    wrapper |> Enzyme.setJsState(jState);
    };

[@bs.module "enzyme"] external mount : Enzyme.node => Enzyme.shallowWrapper = "";
[@bs.send.pipe : Enzyme.shallowWrapper] external simulate : string => Enzyme.shallowWrapper = "simulate";