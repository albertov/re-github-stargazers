
module Error = {
    let component = ReasonReact.statelessComponent("Message.Error");
    let make = (~message=ReasonReact.null, ~header=ReasonReact.null, children) => ReasonReact.({
        ...component,
        render: _self =>
            <div className="ui error message">
                <div className="header">
                    (header)
                </div>
                <p>(message)</p>
                (array(children))
            </div>
    });
};